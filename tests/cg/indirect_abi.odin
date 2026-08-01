package main

Large :: struct {
	first, second, third: u64,
}

large_id :: proc(value: Large) -> Large {
	return value
}

main :: proc() {
	value: Large
	large_id(value)
}
