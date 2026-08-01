package main

import "core:testing"

@(test)
test_entry_first :: proc(t: ^testing.T) {
	_ = t
	value := 17
	assert(value*2 == 34)
}
