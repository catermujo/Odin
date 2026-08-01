package main

import "core:simd"

swizzle_i32 :: proc(x, indices: simd.i32x4) -> simd.i32x4 { return simd.runtime_swizzle(x, indices) }
swizzle_i8 :: proc(x, indices: simd.i8x16) -> simd.i8x16 { return simd.runtime_swizzle(x, indices) }

main :: proc() {
	a: simd.i32x4 = 1
	b: simd.i8x16 = 2
	_ = swizzle_i32(a, a)
	_ = swizzle_i8(b, b)
}
