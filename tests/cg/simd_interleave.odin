package main

import "core:simd"

interleave2 :: proc(x, y: #simd[2]i32) -> simd.i32x4 { return simd.interleave(x, y) }
deinterleave2 :: proc(x: simd.i32x4) -> (#simd[2]i32, #simd[2]i32) { return simd.deinterleave(x, 2) }
interleave8 :: proc(x: #simd[2]i32) -> #simd[16]i32 { return simd.interleave(x, x, x, x, x, x, x, x) }
deinterleave8 :: proc(x: #simd[16]i32) -> (#simd[2]i32, #simd[2]i32, #simd[2]i32, #simd[2]i32, #simd[2]i32, #simd[2]i32, #simd[2]i32, #simd[2]i32) { return simd.deinterleave(x, 8) }

main :: proc() {
	a: #simd[2]i32 = 1
	b: #simd[2]i32 = 2
	c: simd.i32x4 = 3
	d: #simd[16]i32 = 4
	_ = interleave2(a, b)
	_, _ = deinterleave2(c)
	_ = interleave8(a)
	_, _, _, _, _, _, _, _ = deinterleave8(d)
}
