package main

import "core:testing"

State :: struct {
	left:  i64,
	right: i64,
}

@(export)
pressure_locals :: #force_no_inline proc(seed: i64, count: int) -> State {
	sum := seed
	alternate := seed * 2
	state := State{sum, alternate}

	for i in 0 ..< count {
		value := i64(i + 1)
		if i & 1 == 0 {
			sum += value
			alternate -= sum
			state.left = sum
		} else {
			alternate += value
			sum ~= alternate
			state.right = alternate
		}

		if sum > alternate {
			state.left += alternate
		} else {
			state.right += sum
		}
	}

	return State{state.left + sum, state.right + alternate}
}

main :: proc() {
	result := pressure_locals(3, 5)
	assert(result == State{16, -1})
}

@(test)
test_ir_pressure_locals :: proc(t: ^testing.T) {
	_ = t
	result := pressure_locals(3, 5)
	assert(result == State{16, -1})
}
