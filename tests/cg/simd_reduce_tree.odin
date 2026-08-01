package main

import "core:simd"

reduce_add_bisect_f32 :: proc(x: simd.f32x4) -> f32 { return simd.reduce_add_bisect(x) }
reduce_mul_bisect_f32 :: proc(x: simd.f32x4) -> f32 { return simd.reduce_mul_bisect(x) }
reduce_add_pairs_i32 :: proc(x: simd.i32x4) -> i32 { return simd.reduce_add_pairs(x) }
reduce_mul_pairs_i32 :: proc(x: simd.i32x4) -> i32 { return simd.reduce_mul_pairs(x) }

main :: proc() {
	f: simd.f32x4 = 1
	i: simd.i32x4 = 1
	_ = reduce_add_bisect_f32(f)
	_ = reduce_mul_bisect_f32(f)
	_ = reduce_add_pairs_i32(i)
	_ = reduce_mul_pairs_i32(i)
}
