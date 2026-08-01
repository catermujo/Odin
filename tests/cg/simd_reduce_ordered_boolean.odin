package main

import "core:simd"

reduce_add_ordered_f32 :: proc(x: simd.f32x4) -> f32 { return simd.reduce_add_ordered(x) }
reduce_mul_ordered_i32 :: proc(x: simd.i32x4) -> i32 { return simd.reduce_mul_ordered(x) }
reduce_any_u32 :: proc(x: simd.u32x4) -> bool { return simd.reduce_any(x) }
reduce_all_u32 :: proc(x: simd.u32x4) -> bool { return simd.reduce_all(x) }

main :: proc() {
	f: simd.f32x4 = 1
	i: simd.i32x4 = 1
	u: simd.u32x4 = 1
	_ = reduce_add_ordered_f32(f)
	_ = reduce_mul_ordered_i32(i)
	_ = reduce_any_u32(u)
	_ = reduce_all_u32(u)
}
