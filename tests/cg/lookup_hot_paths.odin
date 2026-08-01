package main

import "core:testing"

call_heavy :: proc(value: int) -> int {
	result := value
	for i in 0..<64 {
		result = result + i*3
	}
	return result
}

location_line :: proc(loc := #caller_location) -> int {
	return int(loc.line)
}

@(test)
lookup_hot_paths_test :: proc(t: ^testing.T) {
	_ = t

	total := 0
	for i in 0..<32 {
		total += call_heavy(i)
	}
	assert(total != 0)

	assert(location_line() > 0)
}
