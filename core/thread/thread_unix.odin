#+build linux, darwin, freebsd, openbsd, netbsd
#+private
package thread

import "core:strings"
import "core:sync"
import "core:sys/posix"

import "base:runtime"

_IS_SUPPORTED :: true
// NOTE(tetra): Aligned here because of core/unix/pthread_linux.odin/pthread_t.
// Also see core/sys/darwin/mach_darwin.odin/semaphore_t.
Thread_Os_Specific :: struct #align(16) {
	unix_thread: posix.pthread_t, // NOTE: very large on Darwin, small on Linux.
	start_ok:    sync.Sema,
}
@(private="file")
unix_thread_entry_proc :: proc "c" (data: rawptr) -> rawptr {
	thread := (^Thread)(data)
	thread.lifecycle.id = sync.current_thread_id()

	_unix_wait_for_start(thread)
	_unix_enable_cancellation()
	_thread_execute(thread)
	_thread_mark_done(thread)
	_unix_self_cleanup(thread)
	return nil
}

@(private="file")
_unix_wait_for_start :: proc "contextless" (thread: ^Thread) {
	for (.Started not_in sync.atomic_load(&thread.lifecycle.flags)) {
		sync.wait(&thread.start_ok)
	}
}

@(private="file")
_unix_enable_cancellation :: proc "contextless" () {
	err := posix.pthread_setcancelstate(.ENABLE, nil)
	assert_contextless(err == nil)

	err = posix.pthread_setcanceltype(.ASYNCHRONOUS, nil)
	assert_contextless(err == nil)
}

@(private="file")
_unix_self_cleanup :: proc "contextless" (thread: ^Thread) {
	if .Self_Cleanup not_in sync.atomic_load(&thread.lifecycle.flags) {
		return
	}

	res := posix.pthread_detach(thread.unix_thread)
	assert_contextless(res == nil)
	thread.unix_thread = {}
	_thread_release_memory(thread)
}

@(private="file")
_unix_set_priority :: proc(attrs: ^posix.pthread_attr_t, priority: Thread_Priority) {
	policy: posix.Sched_Policy
	res: posix.Errno
	when ODIN_OS != .NetBSD {
		res = posix.pthread_attr_getschedpolicy(attrs, &policy)
		assert(res == nil)
	}

	params: posix.sched_param
	res = posix.pthread_attr_getschedparam(attrs, &params)
	assert(res == nil)
	low := posix.sched_get_priority_min(policy)
	high := posix.sched_get_priority_max(policy)
	switch priority {
	case .Normal:
	case .Low:
		params.sched_priority = low + 1
		if params.sched_priority >= high {
			params.sched_priority = low
		}
	case .High:
		params.sched_priority = high
	}
	res = posix.pthread_attr_setschedparam(attrs, &params)
	assert(res == nil)
}

//
// Creates a thread which will run the given procedure.
// It then waits for `start` to be called.
//
_create :: proc(procedure: Thread_Proc, priority: Thread_Priority, name: Maybe(string)) -> ^Thread {
	attrs: posix.pthread_attr_t
	if posix.pthread_attr_init(&attrs) != nil {
		return nil // NOTE(tetra, 2019-11-01): POSIX OOM.
	}
	defer posix.pthread_attr_destroy(&attrs)

	stacksize: posix.rlimit
	if res := posix.getrlimit(.STACK, &stacksize); res == .OK && stacksize.rlim_cur > 0 {
		_ = posix.pthread_attr_setstacksize(&attrs, uint(stacksize.rlim_cur))
	}

	res: posix.Errno
	// NOTE(tetra, 2019-11-01): These only fail if their argument is invalid.
	res = posix.pthread_attr_setdetachstate(&attrs, .CREATE_JOINABLE)
	assert(res == nil)
	when ODIN_OS != .NetBSD {
		res = posix.pthread_attr_setinheritsched(&attrs, .EXPLICIT_SCHED)
		assert(res == nil)
	}

	thread := _new_thread(procedure, name)
	if thread == nil {
		return nil
	}

	_unix_set_priority(&attrs, priority)

	if posix.pthread_create(&thread.unix_thread, &attrs, unix_thread_entry_proc, thread) != nil {
		free(thread, thread.ownership.creation_allocator)
		return nil
	}

	return thread
}

_start :: proc(t: ^Thread) {
	sync.atomic_or(&t.lifecycle.flags, { .Started })
	sync.post(&t.start_ok)
}

_is_done :: proc(t: ^Thread) -> bool {
	return .Done in sync.atomic_load(&t.lifecycle.flags)
}

_join :: proc(t: ^Thread) {
	if posix.pthread_equal(posix.pthread_self(), t.unix_thread) {
		return
	}

	// If the previous value was already `Joined`, then we can return.
	if .Joined in sync.atomic_or(&t.lifecycle.flags, {.Joined}) {
		return
	}

	// Prevent non-started threads from blocking main thread with initial wait
	// condition.
	for (.Started not_in sync.atomic_load(&t.lifecycle.flags)) {
		_start(t)
	}

	posix.pthread_join(t.unix_thread, nil)

	t.lifecycle.flags += {.Joined}
}

_join_multiple :: proc(threads: ..^Thread) {
	for t in threads {
		_join(t)
	}
}

_destroy :: proc(t: ^Thread) {
	_join(t)
	t.unix_thread = {}
	_thread_free(t)
}

_terminate :: proc(t: ^Thread, exit_code: int) {
	// NOTE(Feoramund): For thread cancellation to succeed on BSDs and
	// possibly Darwin systems, the thread must call one of the pthread
	// cancelation points at some point after this.
	//
	// The most obvious one of these is `pthread_cancel`, but there is an
	// entire list of functions that act as cancelation points available in the
	// pthreads manual page.
	//
	// This is in contrast to behavior I have seen on Linux where the thread is
	// just terminated.
	posix.pthread_cancel(t.unix_thread)
}

_yield :: proc() {
	posix.sched_yield()
}

_get_name :: proc(thread: ^Thread, allocator: runtime.Allocator, loc: runtime.Source_Code_Location) -> (name: string, err: runtime.Allocator_Error) {
	tid: posix.pthread_t
	if thread == nil {
		tid = posix.pthread_self()
	} else {
		tid = thread.unix_thread
	}
	
	buf : [_MAX_PTHREAD_NAME_LENGTH]u8

	when ODIN_OS == .OpenBSD {
		pthread_get_name_np(tid, raw_data(buf[:]), len(buf))
	} else {
		pthread_getname_np(tid, raw_data(buf[:]), len(buf))
	}

	name, err = strings.clone_from_cstring(cstring(raw_data(buf[:])), allocator, loc)
	return
}

_set_name :: proc(thread: ^Thread) {
	name, ok := thread.name.?
	if !ok {
		return
	}

	when ODIN_OS != .Darwin {
		tid := thread.unix_thread
	}

	// _MAX_PTHREAD_NAME_LENGTH includes terminating null
	buflen := len(name) + 1 < _MAX_PTHREAD_NAME_LENGTH ? len(name) + 1 : _MAX_PTHREAD_NAME_LENGTH
	buf := make([]u8, buflen)
	defer delete(buf)
	copy(buf[:len(buf) - 1], name)

	when ODIN_OS == .Darwin {
		pthread_setname_np(raw_data(buf[:]))
	} else when ODIN_OS == .OpenBSD {
		pthread_set_name_np(tid, raw_data(buf[:]))
	} else when ODIN_OS == .NetBSD {
		pthread_setname_np(tid, "%s", raw_data(buf[:]))
	} else {
		pthread_setname_np(tid, raw_data(buf[:]))
	}
}
