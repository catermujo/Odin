package main

import "reach_helper"

apply :: proc(value: i32, f: proc(i32) -> i32) -> i32 {
	return f(value)
}

test_deterministic_reachability :: proc() {
	key := reach_helper.make_key()
	other := reach_helper.make_key()
	values := make(map[reach_helper.Key]i32)
	defer delete_map(values)
	values[key] = reach_helper.global_value

	assert(values[other] == reach_helper.global_value)
	assert(apply(reach_helper.global_value, reach_helper.transform) == 26)
	assert(apply(reach_helper.global_value, proc(value: i32) -> i32 {
		return value + 1
	}) == 14)
}

main :: proc() {
	test_deterministic_reachability()
}
