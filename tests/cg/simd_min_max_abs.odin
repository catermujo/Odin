package main

import "core:simd"

neg_f32 :: proc(value: simd.f32x4) -> simd.f32x4 { return simd.neg(value) }
abs_i32 :: proc(value: simd.i32x4) -> simd.i32x4 { return simd.abs(value) }
min_f32 :: proc(x, y: simd.f32x4) -> simd.f32x4 { return simd.min(x, y) }
max_u32 :: proc(x, y: simd.u32x4) -> simd.u32x4 { return simd.max(x, y) }

main :: proc() {
	f: simd.f32x4 = 1
	i: simd.i32x4 = 1
	u: simd.u32x4 = 1
	_ = neg_f32(f)
	_ = abs_i32(i)
	_ = min_f32(f, f)
	_ = max_u32(u, u)
}
