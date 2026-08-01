package main

import "core:math/linalg"

Matrix :: linalg.Matrix3f32

State :: struct {
	stack: [dynamic]Matrix,
}

state: State

init_stack :: proc() {
	state.stack = make([dynamic]Matrix, 0, 16)
	append(&state.stack, Matrix(1.0))
}

local_stack_len :: proc() -> int {
	stack := make([dynamic]Matrix, 0, 16)
	append(&stack, Matrix(1.0))
	return len(stack)
}

main :: proc() {
	assert(local_stack_len() == 1)
	init_stack()
	assert(cap(state.stack) >= 16)
	assert(len(state.stack) == 1)
	assert(state.stack[0][0][0] == 1.0)
	assert(state.stack[0][1][1] == 1.0)
	assert(state.stack[0][2][2] == 1.0)
	assert(state.stack[0][0][1] == 0.0)
}
