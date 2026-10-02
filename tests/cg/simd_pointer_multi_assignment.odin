package main

import "core:simd"

swap :: proc(left, right: ^simd.u8x16) {
	left^, right^ = right^, left^
}

main :: proc() {
	left: simd.u8x16 = 1
	right: simd.u8x16 = 2
	swap(&left, &right)
	assert(simd.reduce_and(simd.lanes_eq(left, simd.u8x16(2))) != 0)
	assert(simd.reduce_and(simd.lanes_eq(right, simd.u8x16(1))) != 0)
}
