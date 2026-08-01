package main

import "core:simd"

broadcast :: proc(value: u8) -> simd.u8x16 {
	return value
}

main :: proc() {
	_ = broadcast(7)
}
