package main

Axis :: enum {
	X = -1,
	Y,
	Z,
}

sum_array :: proc(values: [3]int) -> int {
	total := 0
	#unroll for value, index in values {
		total += value + index
	}
	return total
}

sum_explicit_array :: proc(values: [3]int) -> int {
	total := 0
	#unroll(2) for value, index in values {
		total += value + index
	}
	return total
}

sum_string :: proc() -> rune {
	total: rune
	#unroll for value, offset in "aé" {
		total += value + rune(offset)
	}
	return total
}

sum_enumerated_array :: proc(values: [Axis]int) -> int {
	total := 0
	#unroll for value, axis in values {
		total += value + int(axis)
	}
	return total
}

sum_slice :: proc(values: []int) -> int {
	total := 0
	#unroll(2) for value, index in values {
		total += value + index
	}
	return total
}

main :: proc() {
	_ = sum_array({1, 2, 3})
	_ = sum_explicit_array({1, 2, 3})
	_ = sum_string()
	_ = sum_enumerated_array({.X = 1, .Y = 2, .Z = 3})
	values := [3]int{1, 2, 3}
	_ = sum_slice(values[:])
}
