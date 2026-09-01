// Multi-threading operations to spawn threads and thread pools.
package thread

import "core:mem"

import "base:intrinsics"
import "base:runtime"

@(private)
unall :: intrinsics.unaligned_load
@(private)
unals :: intrinsics.unaligned_store

/*
Value, specifying whether `core:thread` functionality is available on the
current platform.
*/
IS_SUPPORTED :: _IS_SUPPORTED

/*
Type for a procedure that will be run in a thread, after that thread has been
started.
*/
Thread_Proc :: #type proc(^Thread)

/*
Maximum number of user arguments for polymorphic thread procedures.
*/
MAX_USER_ARGUMENTS :: 8

/*
Type representing the state/flags of the thread.
*/
Thread_State :: enum u8 {
	Started,
	Joined,
	Done,
	Self_Cleanup,
}

Invocation :: struct {
	fn:   rawptr,
	args: [MAX_USER_ARGUMENTS]rawptr,
}

Launch_State :: struct {
	procedure:  Thread_Proc,
	data: 		rawptr,
	invocation: Invocation,
	user_index: int,
	// Thread's Name/Description that will get set during thread creation
	// it will be set using init_context's allocator to allocate and free a cstring buffer
	// for thread's creation only : do not refer to it, use thread.get_name instead
	name: Maybe(string),
	// The thread context.
	// This field can be assigned to directly, after the thread has been
	// created, but __before__ the thread has been started. This field must
	// not be changed after the thread has started.
	//
	// **Note**: If this field is **not** set, the temp allocator will be managed
	// automatically. If it is set, the allocators must be handled manually.
	//
	// **IMPORTANT**:
	// By default, the thread proc will get the same context as `main()` gets.
	// In this situation, the thread will get a new temporary allocator which
	// will be cleaned up when the thread dies. ***This does NOT happen when
	// `init_context` field is initialized***.
	//
	// If `init_context` is initialized, and `temp_allocator` field is set to
	// the default temp allocator, then `runtime.default_temp_allocator_destroy()`
	// procedure needs to be called from the thread procedure, in order to prevent
	// any memory leaks.
	init_context: Maybe(runtime.Context),
}

Lifecycle_State :: struct {
	flags: bit_set[Thread_State; u8],
	// Thread ID. Depending on the platform, may start out as 0 (zero) until the thread
	// has had a chance to run.
	id: int,
}

Ownership_State :: struct {
	// The allocator used to allocate data for the thread.
	creation_allocator: mem.Allocator,
}

Runtime_State :: struct {
	using launch:    Launch_State,
	using lifecycle: Lifecycle_State,
	using ownership: Ownership_State,
}

/*
Type representing a thread handle and the associated with that thread data.
*/
Thread :: struct {
	using specific: Thread_Os_Specific,
	using state:    Runtime_State,
}

when IS_SUPPORTED {
	#assert(size_of(Thread{}.data) == size_of(rawptr))
}

/*
Type representing priority of a thread.
*/
Thread_Priority :: enum {
	Normal,
	Low,
	High,
}

/*
Create a thread in a suspended state with the given priority.

This procedure creates a thread that will be set to run the procedure
specified by `procedure` parameter with a specified priority. The returned
thread will be in a suspended state, until `start()` procedure is called.

To start the thread, call `start()`. Also the `create_and_start()`
procedure can be called to create and start the thread immediately.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.
*/

create :: proc(procedure: Thread_Proc, priority := Thread_Priority.Normal, name: Maybe(string) = nil) -> ^Thread {
	return _create(procedure, priority, name)
}

/*
Wait for the thread to finish and free all data associated with it.
*/
destroy :: proc(thread: ^Thread) {
	_destroy(thread)
}

/*
Start a suspended thread.
*/
start :: proc(thread: ^Thread) {
	_start(thread)
}

/*
Check if the thread has finished work.
*/
is_done :: proc(thread: ^Thread) -> bool {
	return _is_done(thread)
}

/*
Wait for the thread to finish work.
*/
join :: proc(thread: ^Thread) {
	_join(thread)
}

/*
Wait for all threads to finish work.
*/
join_multiple :: proc(threads: ..^Thread) {
	_join_multiple(..threads)
}

/*
Forcibly terminate a running thread.
*/
terminate :: proc(thread: ^Thread, exit_code: int) {
	_terminate(thread, exit_code)
}

/*
Yield the execution of the current thread to another OS thread or process.
*/
yield :: proc() {
	_yield()
}

/*
Get thread's name/description.

This procedure returns the name of the given thread. If `thread` is `nil`, this procedure returns the name of the calling thread.
OS level errors are silently ignored.

**Note(linux, bsd)**: Because the thread name is stored in as the `cmdline`, if the thread name was not set, the command that has been used to create the process will be used as the name of the thread.

allocates memory for the returned string using provided allocator.
*/
get_name :: proc(thread: ^Thread = nil, allocator := context.temp_allocator, loc := #caller_location) -> (string, runtime.Allocator_Error) {
	return _get_name(thread, allocator, loc)
}

@(private="package")
_start_invocation :: proc(
	procedure: Thread_Proc,
	invocation: Invocation,
	init_context: Maybe(runtime.Context),
	priority: Thread_Priority,
	self_cleanup: bool,
	name: Maybe(string),
) -> ^Thread {
	t := create(procedure, priority, name)
	if t == nil {
		return nil
	}

	t.invocation = invocation
	t.data = rawptr(&t.invocation)
	if self_cleanup {
		intrinsics.atomic_or(&t.lifecycle.flags, {.Self_Cleanup})
	}
	t.init_context = init_context
	start(t)
	return t
}

@(private="package")
_thread_invocation :: proc(thread: ^Thread) -> ^Invocation {
	invocation := cast(^Invocation)thread.data
	assert(invocation == &thread.invocation)
	return invocation
}

/*
Run a procedure on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
run :: proc(fn: proc(), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, name: Maybe(string) = nil) {
	create_and_start(fn, init_context, priority, true, name)
}

/*
Run a procedure with one pointer parameter on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
run_with_data :: proc(data: rawptr, fn: proc(data: rawptr), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, name: Maybe(string) = nil) {
	create_and_start_with_data(data, fn, init_context, priority, true, name)
}

/*
Run a procedure with one polymorphic parameter on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
run_with_poly_data :: proc(data: $T, fn: proc(data: T), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, name: Maybe(string) = nil)
	where size_of(T) <= size_of(rawptr) * MAX_USER_ARGUMENTS {
	create_and_start_with_poly_data(data, fn, init_context, priority, true, name)
}

/*
Run a procedure with two polymorphic parameters on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
run_with_poly_data2 :: proc(arg1: $T1, arg2: $T2, fn: proc(T1, T2), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, name: Maybe(string) = nil)
	where size_of(T1) + size_of(T2) <= size_of(rawptr) * MAX_USER_ARGUMENTS {
	create_and_start_with_poly_data2(arg1, arg2, fn, init_context, priority, true, name)
}

/*
Run a procedure with three polymorphic parameters on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
run_with_poly_data3 :: proc(arg1: $T1, arg2: $T2, arg3: $T3, fn: proc(arg1: T1, arg2: T2, arg3: T3), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, name: Maybe(string) = nil)
	where size_of(T1) + size_of(T2) + size_of(T3) <= size_of(rawptr) * MAX_USER_ARGUMENTS {
	create_and_start_with_poly_data3(arg1, arg2, arg3, fn, init_context, priority, true, name)
}

/*
Run a procedure with four polymorphic parameters on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
run_with_poly_data4 :: proc(arg1: $T1, arg2: $T2, arg3: $T3, arg4: $T4, fn: proc(arg1: T1, arg2: T2, arg3: T3, arg4: T4), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, name: Maybe(string) = nil)
	where size_of(T1) + size_of(T2) + size_of(T3) + size_of(T4) <= size_of(rawptr) * MAX_USER_ARGUMENTS {
	create_and_start_with_poly_data4(arg1, arg2, arg3, arg4, fn, init_context, priority, true, name)
}

/*
Run a procedure on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

If `self_cleanup` is specified, after the thread finishes the execution of the
`fn` procedure, the resources associated with the thread are going to be
automatically freed.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**Do not** dereference the `^Thread` pointer, if this flag is specified.
That includes calling `join`, which needs to dereference ^Thread`.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
create_and_start :: proc(fn: proc(), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, self_cleanup := false, name: Maybe(string) = nil) -> (t: ^Thread) {
	thread_proc :: proc(t: ^Thread) {
		invocation := _thread_invocation(t)
		fn := cast(proc())invocation.fn
		fn()
	}
	invocation := Invocation{}
	invocation.fn = rawptr(fn)
	t = _start_invocation(thread_proc, invocation, init_context, priority, self_cleanup, name)
	return t
}

/*
Run a procedure with one pointer parameter on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

If `self_cleanup` is specified, after the thread finishes the execution of the
`fn` procedure, the resources associated with the thread are going to be
automatically freed.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**Do not** dereference the `^Thread` pointer, if this flag is specified.
That includes calling `join`, which needs to dereference ^Thread`.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
create_and_start_with_data :: proc(data: rawptr, fn: proc(data: rawptr), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, self_cleanup := false, name: Maybe(string) = nil) -> (t: ^Thread) {
	thread_proc :: proc(t: ^Thread) {
		invocation := _thread_invocation(t)
		fn := cast(proc(rawptr))invocation.fn
		data := invocation.args[0]
		fn(data)
	}
	invocation := Invocation{}
	invocation.fn = rawptr(fn)
	invocation.args[0] = data
	t = _start_invocation(thread_proc, invocation, init_context, priority, self_cleanup, name)
	return t
}

/*
Run a procedure with one polymorphic parameter on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

If `self_cleanup` is specified, after the thread finishes the execution of the
`fn` procedure, the resources associated with the thread are going to be
automatically freed.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**Do not** dereference the `^Thread` pointer, if this flag is specified.
That includes calling `join`, which needs to dereference ^Thread`.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
create_and_start_with_poly_data :: proc(data: $T, fn: proc(data: T), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, self_cleanup := false, name: Maybe(string) = nil) -> (t: ^Thread)
	where size_of(T) <= size_of(rawptr) * MAX_USER_ARGUMENTS {
	thread_proc :: proc(t: ^Thread) {
		invocation := _thread_invocation(t)
		fn := cast(proc(T))invocation.fn

		data := unall((^T)(&invocation.args))

		fn(data)
	}
	invocation := Invocation{}
	invocation.fn = rawptr(fn)
	unals((^T)(&invocation.args), data)
	t = _start_invocation(thread_proc, invocation, init_context, priority, self_cleanup, name)
	return t
}

/*
Run a procedure with two polymorphic parameters on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

If `self_cleanup` is specified, after the thread finishes the execution of the
`fn` procedure, the resources associated with the thread are going to be
automatically freed.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**Do not** dereference the `^Thread` pointer, if this flag is specified.
That includes calling `join`, which needs to dereference ^Thread`.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
create_and_start_with_poly_data2 :: proc(arg1: $T1, arg2: $T2, fn: proc(T1, T2), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, self_cleanup := false, name: Maybe(string) = nil) -> (t: ^Thread)
	where size_of(T1) + size_of(T2) <= size_of(rawptr) * MAX_USER_ARGUMENTS {
	thread_proc :: proc(t: ^Thread) {
		invocation := _thread_invocation(t)
		fn := cast(proc(T1, T2))invocation.fn

		ptr := uintptr(&invocation.args)

		arg1 := unall((^T1)(rawptr(ptr)))
		arg2 := unall((^T2)(rawptr(ptr + size_of(T1))))

		fn(arg1, arg2)
	}
	invocation := Invocation{}
	invocation.fn = rawptr(fn)

	ptr := uintptr(&invocation.args)

	unals((^T1)(rawptr(ptr)), arg1)
	unals((^T2)(rawptr(ptr + size_of(T1))), arg2)

	t = _start_invocation(thread_proc, invocation, init_context, priority, self_cleanup, name)
	return t
}

/*
Run a procedure with three polymorphic parameters on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

If `self_cleanup` is specified, after the thread finishes the execution of the
`fn` procedure, the resources associated with the thread are going to be
automatically freed.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**Do not** dereference the `^Thread` pointer, if this flag is specified.
That includes calling `join`, which needs to dereference ^Thread`.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
create_and_start_with_poly_data3 :: proc(arg1: $T1, arg2: $T2, arg3: $T3, fn: proc(arg1: T1, arg2: T2, arg3: T3), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, self_cleanup := false, name: Maybe(string) = nil) -> (t: ^Thread)
	where size_of(T1) + size_of(T2) + size_of(T3) <= size_of(rawptr) * MAX_USER_ARGUMENTS {
	thread_proc :: proc(t: ^Thread) {
		invocation := _thread_invocation(t)
		fn := cast(proc(T1, T2, T3))invocation.fn

		ptr := uintptr(&invocation.args)

		arg1 := unall((^T1)(rawptr(ptr)))
		arg2 := unall((^T2)(rawptr(ptr + size_of(T1))))
		arg3 := unall((^T3)(rawptr(ptr + size_of(T1) + size_of(T2))))

		fn(arg1, arg2, arg3)
	}
	invocation := Invocation{}
	invocation.fn = rawptr(fn)

	ptr := uintptr(&invocation.args)

	unals((^T1)(rawptr(ptr)), arg1)
	unals((^T2)(rawptr(ptr + size_of(T1))), arg2)
	unals((^T3)(rawptr(ptr + size_of(T1) + size_of(T2))), arg3)

	t = _start_invocation(thread_proc, invocation, init_context, priority, self_cleanup, name)
	return t
}

/*
Run a procedure with four polymorphic parameters on a different thread.

This procedure runs the given procedure on another thread. The context
specified by `init_context` will be used as the context in which `fn` is going
to execute. The thread will have priority specified by the `priority` parameter.

If `self_cleanup` is specified, after the thread finishes the execution of the
`fn` procedure, the resources associated with the thread are going to be
automatically freed.

Optionally specify the thread's name/description.
the name/description will be truncated to fit the OS's limit.

**Do not** dereference the `^Thread` pointer, if this flag is specified.
That includes calling `join`, which needs to dereference ^Thread`.

**IMPORTANT**: If `init_context` is specified and the default temporary allocator
is used, the thread procedure needs to call `runtime.default_temp_allocator_destroy()`
in order to free the resources associated with the temporary allocations.
*/
create_and_start_with_poly_data4 :: proc(arg1: $T1, arg2: $T2, arg3: $T3, arg4: $T4, fn: proc(arg1: T1, arg2: T2, arg3: T3, arg4: T4), init_context: Maybe(runtime.Context) = nil, priority := Thread_Priority.Normal, self_cleanup := false, name: Maybe(string) = nil) -> (t: ^Thread)
	where size_of(T1) + size_of(T2) + size_of(T3) + size_of(T4) <= size_of(rawptr) * MAX_USER_ARGUMENTS {
	thread_proc :: proc(t: ^Thread) {
		invocation := _thread_invocation(t)
		fn := cast(proc(T1, T2, T3, T4))invocation.fn

		user_args := mem.slice_to_bytes(invocation.args[:])
		arg1 := (^T1)(raw_data(user_args))^
		arg2 := (^T2)(raw_data(user_args[size_of(T1):]))^
		arg3 := (^T3)(raw_data(user_args[size_of(T1) + size_of(T2):]))^
		arg4 := (^T4)(raw_data(user_args[size_of(T1) + size_of(T2) + size_of(T3):]))^

		fn(arg1, arg2, arg3, arg4)
	}
	invocation := Invocation{}
	invocation.fn = rawptr(fn)

	arg1, arg2, arg3, arg4 := arg1, arg2, arg3, arg4
	user_args := mem.slice_to_bytes(invocation.args[:])

	n := copy(user_args,     mem.ptr_to_bytes(&arg1))
	n += copy(user_args[n:], mem.ptr_to_bytes(&arg2))
	n += copy(user_args[n:], mem.ptr_to_bytes(&arg3))
	_  = copy(user_args[n:], mem.ptr_to_bytes(&arg4))

	t = _start_invocation(thread_proc, invocation, init_context, priority, self_cleanup, name)
	return t
}

@(private="package")
_new_thread :: proc(procedure: Thread_Proc, name: Maybe(string)) -> ^Thread {
	thread := new(Thread)
	if thread == nil {
		return nil
	}

	thread.ownership.creation_allocator = context.allocator
	thread.procedure = procedure
	thread.name = name
	return thread
}

@(private="package")
_thread_exit_context :: proc(init_context: Maybe(runtime.Context)) {
	_maybe_destroy_default_temp_allocator(init_context)
	runtime.run_thread_local_cleaners()
}

@(private="package")
_thread_execute :: proc "contextless" (thread: ^Thread) {
	init_context := thread.init_context
	context = _select_context_for_thread(init_context)
	defer _thread_exit_context(init_context)

	_set_name(thread)
	thread.procedure(thread)
}

@(private="package")
_thread_mark_done :: proc "contextless" (thread: ^Thread) {
	intrinsics.atomic_or(&thread.lifecycle.flags, {.Done})
}

@(private="package")
_thread_release_memory :: proc "contextless" (thread: ^Thread) {
	allocator := thread.ownership.creation_allocator
	context = {}
	free(thread, allocator)
}

@(private="package")
_thread_free :: proc(thread: ^Thread) {
	free(thread, thread.ownership.creation_allocator)
}

_select_context_for_thread :: proc(init_context: Maybe(runtime.Context)) -> runtime.Context {
	ctx, ok := init_context.?
	if !ok {
		return runtime.default_context()
	}

	/*
		NOTE(tetra, 2023-05-31):
			Ensure that the temp allocator is thread-safe when the user provides a specific initial context to use.
			Without this, the thread will use the same temp allocator state as the parent thread, and thus, bork it up.
	*/
	when !ODIN_DEFAULT_TO_NIL_ALLOCATOR {
		if ctx.temp_allocator.procedure == runtime.default_temp_allocator_proc {
			ctx.temp_allocator.data = &runtime.global_default_temp_allocator_data
		}
	}
	return ctx
}

_maybe_destroy_default_temp_allocator :: proc(init_context: Maybe(runtime.Context)) {
	if init_context != nil {
		// NOTE(tetra, 2023-05-31): If the user specifies a custom context for the thread,
		// then it's entirely up to them to handle whatever allocators they're using.
		return
	}

	if context.temp_allocator.procedure == runtime.default_temp_allocator_proc {
		runtime.default_temp_allocator_destroy(auto_cast context.temp_allocator.data)
	}
}
