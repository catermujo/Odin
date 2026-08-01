package main

import "core:testing"

straight_line :: proc() -> int {
	value: int
	value = 7
	value += 5
	return value
}

branch_merge :: proc(select_left: bool) -> int {
	value: int
	if select_left {
		value = 11
	} else {
		value = 13
	}
	return value
}

loop_phi :: proc(count: int) -> int {
	total: int
	for i := 0; i < count; i += 1 {
		total += i
	}
	return total
}

double :: proc(value: int) -> int {
	return value * 2
}

call_between_accesses :: proc() -> int {
	local := 7
	local += double(3)
	return local
}

global_sink: int

global_store_between_accesses :: proc() -> int {
	local := 9
	global_sink = 4
	local += global_sink
	return local
}

mutate_global :: proc() {
	global_sink = 99
}

loaded_snapshot_across_call :: proc() -> int {
	global_sink = 7
	local := global_sink
	mutate_global()
	return local
}

loaded_snapshot_across_store :: proc() -> int {
	global_sink = 11
	local := global_sink
	global_sink = 13
	return local
}

early_return_with_defer :: proc(value: int) -> int {
	result := value
	defer result += 1
	if value < 0 {
		return result
	}
	return result
}

read_pointer :: proc(value: ^int) -> int {
	return value^
}

escaping_address :: proc(value: int) -> int {
	local := value
	return read_pointer(&local)
}

@(test)
scalar_local_promotion_test :: proc(t: ^testing.T) {
	_ = t
	assert(straight_line() == 12)
	assert(branch_merge(true) == 11)
	assert(branch_merge(false) == 13)
	assert(loop_phi(5) == 10)
	assert(call_between_accesses() == 13)
	assert(global_store_between_accesses() == 13)
	assert(loaded_snapshot_across_call() == 7)
	assert(loaded_snapshot_across_store() == 11)
	assert(early_return_with_defer(-2) == -2)
	assert(early_return_with_defer(3) == 3)
	assert(escaping_address(17) == 17)
}

main :: proc() {
	assert(straight_line() == 12)
	assert(branch_merge(true) == 11)
	assert(branch_merge(false) == 13)
	assert(loop_phi(5) == 10)
	assert(call_between_accesses() == 13)
	assert(global_store_between_accesses() == 13)
	assert(loaded_snapshot_across_call() == 7)
	assert(loaded_snapshot_across_store() == 11)
	assert(early_return_with_defer(-2) == -2)
	assert(early_return_with_defer(3) == 3)
	assert(escaping_address(17) == 17)
}
