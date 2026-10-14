#include <atomic>
#include <cstddef>
#undef NDEBUG
#include <cassert>
#include <cstdio>

#define gb_internal static
#include "thread_pool_publication.hpp"

struct CompletingConsumer {
	std::atomic<int> &tasks_left;
	std::atomic<std::ptrdiff_t> value;
	bool accounted_before_visible;

	CompletingConsumer(std::atomic<int> &count)
		: tasks_left(count), value(0), accounted_before_visible(false) {}

	void store(std::ptrdiff_t next, std::memory_order order) {
		value.store(next, order);
		// DUMBAI: A thief may finish immediately after publication, before the producer executes another instruction.
		accounted_before_visible = tasks_left.load(std::memory_order_acquire) == 1;
		tasks_left.fetch_sub(1, std::memory_order_acq_rel);
	}
};

int main() {
	std::atomic<int> tasks_left(0);
	CompletingConsumer consumer(tasks_left);
	thread_pool_publish_task(tasks_left, consumer, std::ptrdiff_t(1));
	assert(consumer.accounted_before_visible);
	assert(tasks_left.load() == 0);
	assert(consumer.value.load() == 1);

	std::atomic<std::ptrdiff_t> bottom(7);
	tasks_left.store(3);
	thread_pool_publish_task(tasks_left, bottom, std::ptrdiff_t(8));
	assert(tasks_left.load() == 4);
	assert(bottom.load() == 8);
	std::puts("Thread pool publication accounting passed");
}
