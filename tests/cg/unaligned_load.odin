package main

import "base:intrinsics"

Pair :: struct {
	first:  u16,
	second: u16,
}

Big :: struct {
	values: [5]u64,
}

load_u16 :: proc(ptr: ^u16) -> u16 {
	return intrinsics.unaligned_load(ptr)
}

load_pair :: proc(ptr: ^Pair) -> Pair {
	return intrinsics.unaligned_load(ptr)
}

load_big :: proc(ptr: ^Big) -> Big {
	return intrinsics.unaligned_load(ptr)
}

main :: proc() {
	value: u16
	pair: Pair
	big: Big
	_ = load_u16(&value)
	_ = load_pair(&pair)
	_ = load_big(&big)
}
