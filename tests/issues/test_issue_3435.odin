package main

import "base:runtime"
import "core:mem"
import "core:testing"
import "core:time"

@test
test_issue_3435 :: proc(t: ^testing.T) {
	testing.set_fail_timeout(t, time.Second)
	allocator: mem.Buddy_Allocator
	data := runtime.make_aligned([]byte, 64, 32)
	defer delete(data)

	// mem.buddy_allocator_init(&allocator, data, 32)

	// Bypass the assertion that would normally keep this from happening by
	// manually putting the allocator together.
	allocator.head = cast(^mem.Buddy_Block)raw_data(data)
	allocator.head.size = len(data)
	allocator.head.is_free = true
	allocator.tail = mem.buddy_block_next(allocator.head)
	allocator.alignment = 32

	context.allocator = mem.buddy_allocator(&allocator)

	// Three allocations in the space above is all that's needed to reproduce
	// the bug seen in #3435; this is the most minimal reproduction possible.
	a, ea := make([]u8, 1)
	testing.expect(t, ea == nil)
	testing.expect(t, len(a) == 1)

	b, eb := make([]u8, 1)
	testing.expect(t, eb == .Out_Of_Memory)
	testing.expect(t, len(b) == 0)

	c, ec := make([]u8, 1)
	testing.expect(t, ec == .Out_Of_Memory)
	testing.expect(t, len(c) == 0)

	// With the bugfix in place, the allocator should be sensible enough to not
	// fall into an infinite loop anymore, even if the assertion is disabled.
}
