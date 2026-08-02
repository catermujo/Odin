package main

import "base:intrinsics"

Large :: [1025]u32

Ternary_State :: struct {
	dst:    Large,
	before: Large,
	after:  Large,
}

large_result: Large
attempts: u32
observed: u32
ternary_state: Ternary_State

make_large :: proc() -> (Large, u32) {
	defer {
		observed = large_result[0]
	}
	result: Large
	result[0] = 7
	result[1024] = 99
	return result, 42
}

make_optional_large :: proc() -> (Large, u32, bool) #optional_ok {
	result: Large
	result[0] = 11
	result[1024] = 101
	return result, 84, true
}

assign_ternary_large :: proc(flag: bool) {
	ternary_state.dst = flag ? ternary_state.after : ternary_state.before
}

assign_large_candidates :: proc() {
	best: Large
	best_score: u32
	candidate: Large
	score: u32

	for i in 0..<4 {
		candidate[0] = u32(i) + 10
		candidate[1024] = u32(i) + 100
		score = u32(i) + 1000
		best, best_score = candidate, score
		assert(best[0] == candidate[0])
		assert(best[1024] == candidate[1024])
		assert(best_score == score)
	}
}

assign_volatile_large :: proc() {
	source: Large
	destination: Large
	score: u32

	source[0] = 19
	source[1024] = 109
	destination, score = intrinsics.volatile_load(&source), 2000
	assert(destination[0] == 19)
	assert(destination[1024] == 109)
	assert(score == 2000)
}

swap_large_values :: proc() {
	left: Large
	right: Large

	left[0] = 23
	left[1024] = 113
	right[0] = 29
	right[1024] = 119
	left, right = right, left
	assert(left[0] == 29)
	assert(left[1024] == 119)
	assert(right[0] == 23)
	assert(right[1024] == 113)
}

main :: proc() {
	large_result[0] = 1
	large_result, attempts = make_large()
	assert(large_result[0] == 7)
	assert(large_result[1024] == 99)
	assert(attempts == 42)
	assert(observed == 1)

	large_result, attempts = make_optional_large()
	assert(large_result[0] == 11)
	assert(large_result[1024] == 101)
	assert(attempts == 84)

	_, attempts = make_large()
	assert(attempts == 42)

	ternary_state.before[0] = 13
	ternary_state.before[1024] = 103
	ternary_state.after[0] = 17
	ternary_state.after[1024] = 107
	assign_ternary_large(true)
	assert(ternary_state.dst[0] == 17)
	assert(ternary_state.dst[1024] == 107)
	assign_ternary_large(false)
	assert(ternary_state.dst[0] == 13)
	assert(ternary_state.dst[1024] == 103)
	ternary_state.before = true ? ternary_state.before : ternary_state.after
	assert(ternary_state.before[0] == 13)
	assert(ternary_state.before[1024] == 103)

	assign_large_candidates()
	assign_volatile_large()
	swap_large_values()
}
