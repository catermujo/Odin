package main

import "core:fmt"

check_context :: proc(expected: int) {
	assert(context.user_index == expected)
}

main :: proc() {
	assert(context.user_index == 0)
	for i in 0..<2 {
		defer check_context(0)
		defer free_all(context.temp_allocator)
		context.allocator = context.temp_allocator
		context.user_index = i + 1
		text := fmt.tprintf("%d", i)
		assert(text == "0" || text == "1")
	}
	assert(context.user_index == 0)
}
