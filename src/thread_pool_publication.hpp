#include <atomic>

template <typename Counter, typename Bottom, typename Index>
gb_internal void thread_pool_publish_task(Counter &tasks_left, Bottom &bottom, Index next_bottom) {
	// DUMBAI: Count work before a thief can observe it, so completion and idle waits never precede accounting.
	tasks_left.fetch_add(1, std::memory_order_release);
	std::atomic_thread_fence(std::memory_order_release);
	bottom.store(next_bottom, std::memory_order_relaxed);
}
