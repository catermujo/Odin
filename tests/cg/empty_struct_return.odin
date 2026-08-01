package main

Empty :: struct {}

empty :: proc() -> Empty {
	return {}
}

forward :: proc() -> Empty {
	return empty()
}

empty_assignment :: proc(dst: ^Empty) {
	dst^ = forward()
}

main :: proc() {
	empty_assignment(nil)
}
