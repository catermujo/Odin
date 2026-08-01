package main

State :: enum {
	Minus = -2,
	Zero,
	Seven = 7,
}

sum_states :: proc() -> int {
	total := 0
	#unroll for state, ordinal in State {
		total += int(state) + ordinal
	}
	return total
}

main :: proc() {
	_ = sum_states()
}
