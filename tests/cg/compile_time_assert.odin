package main

assert_size :: proc(value: int) -> int {
	#assert(size_of(int) > 0)
	return value
}

main :: proc() {
	_ = assert_size(1)
}
