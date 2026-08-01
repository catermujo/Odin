package main

Pair :: struct {
	left, right: int,
}

cleanup :: proc(pair: Pair) {
}

open :: proc(pair: Pair) -> Pair #scope_exit(.explicit, cleanup(pair)) {
	return pair
}

sum :: proc(pair: Pair) -> int {
	result := 0
	with scoped := open(pair) {
		result = scoped.left + scoped.right
	}
	return result
}

main :: proc() {
	_ = sum({2, 3})
}
