package main

Axis :: enum i8 {
	first = -1,
	second,
	third,
}

Moves :: [Axis]int

make_moves :: proc(value: int) -> Moves {
	return {
		.first = value,
		.second = 2,
		.third = 3,
	}
}

make_range_moves :: proc(value: int) -> Moves {
	return {
		min(Axis) ..= .third = value,
	}
}

move_sum :: proc(value: int) -> int {
	moves := make_moves(value)
	return moves[.first] + moves[.second] + moves[.third]
}

range_move_sum :: proc(value: int) -> int {
	moves := make_range_moves(value)
	return moves[.first] + moves[.second] + moves[.third]
}

main :: proc() {
	_ = move_sum(1)
	_ = range_move_sum(1)
}
