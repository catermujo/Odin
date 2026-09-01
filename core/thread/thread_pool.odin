package thread

/*
	thread.Pool
	Copyright 2022 eisbehr
	Made available under Odin's license.
*/

import "core:container/queue"
import "core:mem"
import "core:sync"

import "base:intrinsics"

Task_Proc :: #type proc(task: Task)

Task :: struct {
	procedure:  Task_Proc,
	data:       rawptr,
	user_index: int,
	allocator:  mem.Allocator,
}

Thread_Init_Proc :: #type proc(thread: ^Thread, user_data: rawptr)
Thread_Fini_Proc :: #type proc(thread: ^Thread, user_data: rawptr)

// Do not access the pool's members directly while the pool threads are running,
// since they use different kinds of locking and mutual exclusion devices.
// Careless access can and will lead to nasty bugs. Once initialized, the
// pool's memory address is not allowed to change until it is destroyed.
Pool :: struct {
	allocator:     mem.Allocator,
	mutex:         sync.Mutex,
	sem_available: sync.Sema,

	// the following values are atomic
	num_waiting:       int,
	num_in_processing: int,
	num_outstanding:   int, // num_waiting + num_in_processing
	num_done:          int,
	// end of atomics

	// called once per thread at startup
	thread_init_proc: Thread_Init_Proc,
	thread_init_data: rawptr,
	// called once per thread at shutdown
	thread_fini_proc: Thread_Fini_Proc,
	thread_fini_data: rawptr,

	is_running: bool,

	threads: []Pool_Thread,


	tasks:      queue.Queue(Task),
	tasks_done: [dynamic]Task,
}

Pool_Thread_Data :: struct {
	pool: ^Pool,
	task: Task,
}

Pool_Thread :: struct {
	thread: ^Thread,
	data:   Pool_Thread_Data,
}

@(private="file")
pool_thread_runner :: proc(t: ^Thread) {
	data := cast(^Pool_Thread_Data)t.data
	pool_thread_initialize(t, data)
	pool_thread_run(data)
	pool_thread_finalize(t, data)
}

@(private="file")
pool_thread_initialize :: proc(thread: ^Thread, data: ^Pool_Thread_Data) {
	pool := data.pool
	if pool.thread_init_proc != nil {
		pool.thread_init_proc(thread, pool.thread_init_data)
	}
}

@(private="file")
pool_thread_run :: proc(data: ^Pool_Thread_Data) {
	pool := data.pool
	for intrinsics.atomic_load(&pool.is_running) {
		sync.wait(&pool.sem_available)
		pool_thread_run_one(data)
	}
}

@(private="file")
pool_thread_run_one :: proc(data: ^Pool_Thread_Data) {
	pool := data.pool
	if task, ok := pool_pop_waiting(pool); ok {
		data.task = task
		pool_do_work(pool, task)
		sync.guard(&pool.mutex)
		data.task = {}
	}
}

@(private="file")
pool_thread_finalize :: proc(thread: ^Thread, data: ^Pool_Thread_Data) {
	pool := data.pool
	if pool.thread_fini_proc != nil {
		pool.thread_fini_proc(thread, pool.thread_fini_data)
	}

	sync.post(&pool.sem_available, 1)
}

// Once initialized, the pool's memory address is not allowed to change until
// it is destroyed.
//
// The thread pool requires an allocator which it either owns, or which is thread safe.
pool_init :: proc(
	pool: ^Pool,
	allocator: mem.Allocator,
	thread_count: int,
	init_proc: Thread_Init_Proc = nil,
	init_data: rawptr = nil,
	fini_proc: Thread_Init_Proc = nil,
	fini_data: rawptr = nil,
){
	context.allocator = allocator
	pool.allocator = allocator
	queue.init(&pool.tasks)
	pool.tasks_done = make([dynamic]Task)
	pool.threads    = make([]Pool_Thread, max(thread_count, 1))

	pool.thread_init_proc = init_proc
	pool.thread_fini_proc = fini_proc
	pool.thread_init_data = init_data
	pool.thread_fini_data = fini_data

	pool.is_running = true

	for i in 0..<len(pool.threads) {
		pool_thread := &pool.threads[i]
		pool_thread.thread = create(pool_thread_runner)
		pool_thread.data.pool = pool
		pool_thread.thread.data = &pool_thread.data
	}
}

pool_destroy :: proc(pool: ^Pool) {
	for &pool_thread in pool.threads {
		destroy(pool_thread.thread)
	}

	queue.destroy(&pool.tasks)
	delete(pool.tasks_done)
	delete(pool.threads, pool.allocator)
}

pool_start :: proc(pool: ^Pool) {
	for t in pool.threads {
		start(t.thread)
	}
}

// Finish tasks that have already started processing, then shut down all pool
// threads. Might leave over waiting tasks, any memory allocated for the
// user data of those tasks will not be freed.
pool_join :: proc(pool: ^Pool) {
	intrinsics.atomic_store(&pool.is_running, false)
	sync.post(&pool.sem_available, len(pool.threads))

	yield()

	unstarted_count: int
	for pool_thread in pool.threads {
		flags := intrinsics.atomic_load(&pool_thread.thread.lifecycle.flags)
		if .Started not_in flags {
			unstarted_count += 1
		}
	}

	// most likely the user forgot to call `pool_start`
	// exit here, so we don't hang forever
	if len(pool.threads) == unstarted_count {
		return
	}

	started_count: int
	for started_count < len(pool.threads) {
		started_count = 0
		for pool_thread in pool.threads {
			flags := intrinsics.atomic_load(&pool_thread.thread.lifecycle.flags)
			if .Started in flags {
				started_count += 1
				if .Joined not_in flags {
					join(pool_thread.thread)
				}
			}
		}
	}
}

// Add a task to the thread pool.
//
// Tasks can be added from any thread, not just the thread that created
// the thread pool. You can even add tasks from inside other tasks.
//
// Each task also needs an allocator which it either owns, or which is thread
// safe.
//
// Completed tasks remain in the pool until removed with `pool_pop_done`.
// When reusing the pool, call it once for every task added.
pool_add_task :: proc(pool: ^Pool, allocator: mem.Allocator, procedure: Task_Proc, data: rawptr, user_index: int = 0) {
	sync.guard(&pool.mutex)

	queue.push_back(&pool.tasks, Task{
		procedure  = procedure,
		data       = data,
		user_index = user_index,
		allocator  = allocator,
	})
	intrinsics.atomic_add(&pool.num_waiting, 1)
	intrinsics.atomic_add(&pool.num_outstanding, 1)
	sync.post(&pool.sem_available, 1)
}

@(private="file")
pool_stop_thread :: proc(pool: ^Pool, thread_index: int, thread: ^Thread, data: ^Pool_Thread_Data, exit_code: int) {
	terminate(thread, exit_code)
	pool_record_done_locked(pool, data.task)
	destroy(thread)

	data.task = {}
	replacement := create(pool_thread_runner)
	replacement.data = data
	pool.threads[thread_index].thread = replacement
	start(replacement)
}

// Forcibly stop a running task by its user index.
//
// This will terminate the underlying thread. Ideally, you should use some
// means of communication to stop a task, as thread termination may leave
// resources unclaimed.
//
// The thread will be restarted to accept new tasks.
//
// Returns true if the task was found and terminated.
pool_stop_task :: proc(pool: ^Pool, user_index: int, exit_code: int = 1) -> bool {
	sync.guard(&pool.mutex)

	for i in 0..<len(pool.threads) {
		t := pool.threads[i].thread
		data := &pool.threads[i].data
		if data.task.user_index == user_index && data.task.procedure != nil {
			pool_stop_thread(pool, i, t, data, exit_code)
			return true
		}
	}

	return false
}

// Forcibly stop all running tasks.
//
// The same notes from `pool_stop_task` apply here.
pool_stop_all_tasks :: proc(pool: ^Pool, exit_code: int = 1) {
	sync.guard(&pool.mutex)

	for i in 0..<len(pool.threads) {
		t := pool.threads[i].thread
		data := &pool.threads[i].data
		if data.task.procedure != nil {
			pool_stop_thread(pool, i, t, data, exit_code)
		}
	}
}

// Force the pool to stop all of its threads and put it into a where
// it will no longer run any more tasks.
//
// The pool must still be destroyed after this.
pool_shutdown :: proc(pool: ^Pool, exit_code: int = 1) {
	intrinsics.atomic_store(&pool.is_running, false)
	sync.guard(&pool.mutex)

	for i in 0..<len(pool.threads) {
		t := pool.threads[i].thread
		terminate(t, exit_code)

		data := &pool.threads[i].data
		if data.task.procedure != nil {
			pool_record_done_locked(pool, data.task)
		}
	}
}

// Number of tasks waiting to be processed. Only informational, mostly for
// debugging. Don't rely on this value being consistent with other num_*
// values.
pool_num_waiting :: #force_inline proc(pool: ^Pool) -> int {
	return intrinsics.atomic_load(&pool.num_waiting)
}

// Number of tasks currently being processed. Only informational, mostly for
// debugging. Don't rely on this value being consistent with other num_*
// values.
pool_num_in_processing :: #force_inline proc(pool: ^Pool) -> int {
	return intrinsics.atomic_load(&pool.num_in_processing)
}

// Outstanding tasks are all tasks that are not done, that is, tasks that are
// waiting, as well as tasks that are currently being processed. Only
// informational, mostly for debugging. Don't rely on this value being
// consistent with other num_* values.
pool_num_outstanding :: #force_inline proc(pool: ^Pool) -> int {
	return intrinsics.atomic_load(&pool.num_outstanding)
}

// Number of tasks which are done processing. Only informational, mostly for
// debugging. Don't rely on this value being consistent with other num_*
// values.
pool_num_done :: #force_inline proc(pool: ^Pool) -> int {
	return intrinsics.atomic_load(&pool.num_done)
}

// If tasks are only being added from one thread, and this procedure is being
// called from that same thread, it will reliably tell if the thread pool is
// empty or not. Empty in this case means there are no tasks waiting, being
// processed, or _done_.
pool_is_empty :: #force_inline proc(pool: ^Pool) -> bool {
	return pool_num_outstanding(pool) == 0 && pool_num_done(pool) == 0
}

// Mostly for internal use.
pool_pop_waiting :: proc(pool: ^Pool) -> (task: Task, got_task: bool) {
	sync.guard(&pool.mutex)

	if queue.len(pool.tasks) != 0 {
		intrinsics.atomic_sub(&pool.num_waiting, 1)
		intrinsics.atomic_add(&pool.num_in_processing, 1)
		task = queue.pop_front(&pool.tasks)
		got_task = true
	}

	return
}

// Remove and return the next completed task, if one is available.
//
// The caller is responsible for processing the result and releasing any
// resources associated with the task.
pool_pop_done :: proc(pool: ^Pool) -> (task: Task, got_task: bool) {
	sync.guard(&pool.mutex)

	if len(pool.tasks_done) != 0 {
		task = pop_front(&pool.tasks_done)
		got_task = true
		intrinsics.atomic_sub(&pool.num_done, 1)
	}

	return
}

@(private="file")
pool_execute_task :: proc(task: Task) {
	context.allocator = task.allocator
	task.procedure(task)
}

@(private="file")
pool_record_done_locked :: proc(pool: ^Pool, task: Task) {
	append(&pool.tasks_done, task)
	intrinsics.atomic_add(&pool.num_done, 1)
	intrinsics.atomic_sub(&pool.num_outstanding, 1)
	intrinsics.atomic_sub(&pool.num_in_processing, 1)
}

// Mostly for internal use.
pool_do_work :: proc(pool: ^Pool, task: Task) {
	pool_execute_task(task)
	sync.guard(&pool.mutex)
	pool_record_done_locked(pool, task)
}

// Process the rest of the tasks, also use this thread for processing, then join
// all the pool threads.
//
// Completed tasks are not removed. Retrieve each one with `pool_pop_done`.
// The pool cannot be restarted after this procedure returns.
pool_finish :: proc(pool: ^Pool) {
	for task in pool_pop_waiting(pool) {
		pool_do_work(pool, task)
	}
	pool_join(pool)
}
