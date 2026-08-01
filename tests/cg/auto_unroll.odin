package main

sum_constant_range :: proc() -> int {
	total := 0
	for value, index in 0..<4 {
		total += value + index
	}
	return total
}

sum_inclusive_range :: proc() -> int {
	total := 0
	for value in -1..=1 {
		total += value
	}
	return total
}

sum_negative_range :: proc() -> int {
	total := 0
	for value in -2..<2 {
		total += value
	}
	return total
}

sum_eight_iterations :: proc() -> int {
	total := 0
	for value in 0..<8 {
		total += value
	}
	return total
}

sum_conditional_range :: proc() -> int {
	total := 0
	for value in 0..<4 {
		if value % 2 == 0 {
			total += value
		}
	}
	return total
}

sum_nested_range :: proc() -> int {
	total := 0
	for outer in 0..<2 {
		for inner in 0..<2 {
			total += outer + inner
		}
	}
	return total
}

sum_dynamic_range :: proc(count: int) -> int {
	total := 0
	for value in 0..<count {
		total += value
	}
	return total
}

sum_break :: proc() -> int {
	total := 0
	for value in 0..<4 {
		if value == 2 {
			break
		}
		total += value
	}
	return total
}

sum_continue :: proc() -> int {
	total := 0
	for value in 0..<4 {
		if value == 2 {
			continue
		}
		total += value
	}
	return total
}

main :: proc() {
	if sum_constant_range() != 12 {
		panic("CG auto-unroll constant range regression")
	}
	if sum_inclusive_range() != 0 {
		panic("CG auto-unroll inclusive range regression")
	}
	if sum_negative_range() != -2 {
		panic("CG auto-unroll negative range regression")
	}
	if sum_eight_iterations() != 28 {
		panic("CG auto-unroll cutoff regression")
	}
	if sum_conditional_range() != 2 {
		panic("CG auto-unroll conditional range regression")
	}
	if sum_nested_range() != 4 {
		panic("CG auto-unroll nested range regression")
	}
	if sum_dynamic_range(4) != 6 {
		panic("CG auto-unroll dynamic range regression")
	}
	if sum_break() != 1 {
		panic("CG auto-unroll break regression")
	}
	if sum_continue() != 4 {
		panic("CG auto-unroll continue regression")
	}
}
