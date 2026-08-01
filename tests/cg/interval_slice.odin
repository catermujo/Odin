package main

interval_sum :: proc() -> int {
	sum := 0
	for value, index in 2..<4 {
		sum += value + index
	}
	return sum
}

slice_len :: proc(value: []u32) -> int {
	return len(value)
}

subslice_len :: proc(value: []u32, low: int) -> int {
	return len(value[low:])
}

prefix_len :: proc(value: []u32, high: int) -> int {
	return len(value[:high])
}

main :: proc() {
	values := []u32{3, 5}
	interval_sum()
	slice_len(values)
	subslice_len(values, 1)
	prefix_len(values, 1)
}
