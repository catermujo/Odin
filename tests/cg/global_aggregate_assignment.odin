package main

State :: struct {
	values: [2]i32,
}

state: State

make_state :: proc "contextless" () -> State {
	return {values = {1, 2}}
}

main :: proc() {
	state = make_state()
}
