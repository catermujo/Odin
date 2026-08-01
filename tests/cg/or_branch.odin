package main

even :: proc(value: int) -> (int, bool) {
	return value, value % 2 == 0
}

sum_even :: proc(values: []int) -> int {
	total := 0
	outer: for value in values {
		even_value := even(value) or_continue outer
		total += even_value
	}
	return total
}

sum_until_negative :: proc(values: []int) -> int {
	total := 0
	for value in values {
		(value >= 0) or_break
		total += value
	}
	return total
}

main :: proc() {
	values := []int{1, 2, 3, 4, -1}
	_ = sum_even(values)
	_ = sum_until_negative(values)
}
