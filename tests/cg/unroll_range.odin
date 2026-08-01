package main

sum :: proc() -> int {
	total := 0
	#unroll for i in 0..<4 {
		total += i
	}
	return total
}

sum_with_ordinal :: proc() -> int {
	total := 0
	#unroll for value, ordinal in -1 ..= 1 {
		total += value + ordinal
	}
	return total
}

main :: proc() {
	_ = sum()
	_ = sum_with_ordinal()
}
