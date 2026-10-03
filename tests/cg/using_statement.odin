#+feature using-stmt
package main

Pair :: struct {
	left:  int,
	right: int,
}

using_value :: proc() -> int {
	pair := Pair{10, 20}
	using pair
	return left + right
}

using_pointer :: proc(pair: ^Pair) -> int {
	using pair
	left += 1
	right = left + right
	return right
}

main :: proc() {
	pair := Pair{1, 2}
	_ = using_value()
	_ = using_pointer(&pair)
}
