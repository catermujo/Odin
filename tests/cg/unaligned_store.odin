package main

import "base:intrinsics"

store :: proc(dst: ^u32, value: u32) {
	intrinsics.unaligned_store(dst, value)
}

main :: proc() {
	value: u32
	store(&value, 0x030201)
}
