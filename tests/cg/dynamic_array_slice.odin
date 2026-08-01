package main

@(export)
dynamic_array_slice_len :: proc(values: [dynamic]i64) -> int {
	slice := values[:]
	return len(slice)
}

main :: proc() {
}
