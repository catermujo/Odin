package main

count_args :: proc(values: ..i32) -> int {
	return len(values)
}

main :: proc() {
	_ = count_args(1, 2, 3)
}
