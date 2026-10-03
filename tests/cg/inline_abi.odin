package main

twice :: #force_inline proc(value: u32) -> u32 {
	return value + value
}

main :: proc() {
	twice(21)
}
