package main

Pair :: struct {
	left, right: u32,
}

pair_id :: proc(value: Pair) -> Pair {
	return value
}

main :: proc() {
	pair: Pair
	pair_id(pair)
}
