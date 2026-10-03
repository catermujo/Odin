package main

import "base:runtime"

State :: struct {
	value: int,
	guard: int,
}

state := &State{41, 1}

@(fini)
check_global_pointer_compound_literal :: proc "contextless" () {
	context = runtime.default_context()
	assert(state.value == 42)
	assert(state.guard == 1)
}

main :: proc() {
	assert(state.value == 41)
	state.value += 1
}
