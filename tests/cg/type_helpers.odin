package main

import "base:intrinsics"

Pair :: struct {
	left, right: int,
}

main :: proc() {
	first := Pair{10, 20}
	second := Pair{10, 20}
	equal := intrinsics.type_equal_proc(Pair)
	if typeid_of(Pair) != typeid_of(Pair) || !equal(&first, &second) {
		return
	}
}
