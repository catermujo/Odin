#+build windows
#+private
package thread

import "core:sync"
import win32 "core:sys/windows"
import "core:unicode/utf16"

import "base:runtime"

_IS_SUPPORTED :: true

//NOTE(peperronii): this is the system limit for windows api call, not specific to thread description
_THREAD_DESCRIPTION_LENGTH :: 32_767

Thread_Os_Specific :: struct {
	win32_thread:    win32.HANDLE,
	win32_thread_id: win32.DWORD,
	mutex:           sync.Mutex,
	start_ok:        sync.Sema,
}

_thread_priority_map := [Thread_Priority]i32{
	.Normal = 0,
	.Low = -2,
	.High = +2,
}
@(private="file")
windows_thread_entry_proc :: proc "system" (data: rawptr) -> win32.DWORD {
	thread := (^Thread)(data)

	_windows_wait_for_start(thread)
	_thread_execute(thread)
	_thread_mark_done(thread)
	_windows_self_cleanup(thread)
	return 0
}

@(private="file")
_windows_wait_for_start :: proc "contextless" (thread: ^Thread) {
	for (.Started not_in sync.atomic_load(&thread.lifecycle.flags)) {
		sync.wait(&thread.start_ok)
	}
}

@(private="file")
_windows_self_cleanup :: proc "contextless" (thread: ^Thread) {
	if .Self_Cleanup not_in sync.atomic_load(&thread.lifecycle.flags) {
		return
	}

	win32.CloseHandle(thread.win32_thread)
	thread.win32_thread = win32.INVALID_HANDLE
	_thread_release_memory(thread)
}

@(private="file")
_windows_set_priority :: proc(thread: win32.HANDLE, priority: Thread_Priority) {
	ok := win32.SetThreadPriority(thread, _thread_priority_map[priority])
	assert(ok == true)
}

_create :: proc(procedure: Thread_Proc, priority: Thread_Priority, name: Maybe(string)) -> ^Thread {
	win32_thread_id: win32.DWORD
	thread := _new_thread(procedure, name)
	if thread == nil {
		return nil
	}

	win32_thread := win32.CreateThread(nil, 0, windows_thread_entry_proc, thread, win32.CREATE_SUSPENDED, &win32_thread_id)
	if win32_thread == nil {
	free(thread, thread.ownership.creation_allocator)
		return nil
	}
	thread.win32_thread    = win32_thread
	thread.win32_thread_id = win32_thread_id
	thread.lifecycle.id = int(win32_thread_id)

	_windows_set_priority(win32_thread, priority)

	return thread
}

_start :: proc(t: ^Thread) {
	sync.guard(&t.mutex)
	t.lifecycle.flags += {.Started}
	win32.ResumeThread(t.win32_thread)
}

_is_done :: proc(t: ^Thread) -> bool {
	// NOTE(tetra, 2019-10-31): Apparently using wait_for_single_object and
	// checking if it didn't time out immediately, is not good enough,
	// so we do it this way instead.
	return .Done in sync.atomic_load(&t.lifecycle.flags)
}

_join :: proc(t: ^Thread) {
	sync.guard(&t.mutex)

	if .Joined in t.lifecycle.flags || t.win32_thread == win32.INVALID_HANDLE {
		return
	}

	for (.Started not_in sync.atomic_load(&t.lifecycle.flags)) {
		_start(t)
	}

	win32.WaitForSingleObject(t.win32_thread, win32.INFINITE)
	win32.CloseHandle(t.win32_thread)
	t.win32_thread = win32.INVALID_HANDLE

	t.lifecycle.flags += {.Joined}
}

_join_multiple :: proc(threads: ..^Thread) {
	MAXIMUM_WAIT_OBJECTS :: 64

	handles: [MAXIMUM_WAIT_OBJECTS]win32.HANDLE

	for k := 0; k < len(threads); k += MAXIMUM_WAIT_OBJECTS {
		count := min(len(threads) - k, MAXIMUM_WAIT_OBJECTS)
		j := 0
		for i in 0..<count {
			handle := threads[i+k].win32_thread
			if handle != win32.INVALID_HANDLE {
				handles[j] = handle
				j += 1
			}
		}
		win32.WaitForMultipleObjects(u32(j), &handles[0], true, win32.INFINITE)
	}

	for t in threads {
		win32.CloseHandle(t.win32_thread)
		t.win32_thread = win32.INVALID_HANDLE
		t.lifecycle.flags += {.Joined}
	}
}

_destroy :: proc(thread: ^Thread) {
	_join(thread)
	_thread_free(thread)
}

_terminate :: proc(thread: ^Thread, exit_code: int) {
	win32.TerminateThread(thread.win32_thread, u32(exit_code))
}

_yield :: proc() {
	win32.SwitchToThread()
}

_get_name :: proc(thread: ^Thread, allocator: runtime.Allocator, loc: runtime.Source_Code_Location) -> (name: string, err: runtime.Allocator_Error) {
	t_handle: win32.HANDLE
	if thread == nil {
		t_handle = win32.GetCurrentThread()
	} else {
		t_handle = thread.win32_thread
	}
	
	buf: win32.PWSTR

	hr := win32.GetThreadDescription(t_handle, &buf)

	if win32.SUCCEEDED(hr) {
		defer win32.LocalFree(rawptr(buf))
		name, err = win32.wstring_to_utf8(buf, -1, allocator)
	}
	return
}

_set_name :: proc(thread: ^Thread) {
	name, ok := thread.name.?
	if !ok {
		return
	}

	t_handle := thread.win32_thread

	// _THREAD_DESCRIPTION_LENGTH includes terminating null
	buflen := len(name) + 1 < _THREAD_DESCRIPTION_LENGTH ? len(name) + 1 : _THREAD_DESCRIPTION_LENGTH
	buf :=  make([]u16, buflen)
	defer delete(buf)
	utf16.encode_string(buf[:len(buf) - 1], name)
	win32.SetThreadDescription(t_handle, cstring16(raw_data(buf[:])))
}
