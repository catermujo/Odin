package main

invert :: proc(value: uintptr) -> uintptr {
	return ~value
}

main :: proc() {
	_ = invert(0)
}
