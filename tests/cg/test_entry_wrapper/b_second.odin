package main

import "core:testing"

@(test)
test_entry_second :: proc(t: ^testing.T) {
	_ = t
	value := 29
	assert(value+13 == 42)
}
