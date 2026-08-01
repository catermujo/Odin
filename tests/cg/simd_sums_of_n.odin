package main

import "core:simd"

sums2_i32 :: proc(x: simd.i32x4) -> #simd[2]i32 { return simd.sums_of_n(x, 2) }
sums4_f32 :: proc(x: simd.f32x8) -> #simd[2]f32 { return simd.sums_of_n(x, 4) }
sums4_scalar_f32 :: proc(x: simd.f32x4) -> f32 { return simd.sums_of_n(x, 4) }
sums4_i8x64 :: proc(x: #simd[64]i8) -> #simd[16]i8 { return simd.sums_of_n(x, 4) }

main :: proc() {
	i: simd.i32x4 = 1
	f4: simd.f32x4 = 1
	f8: simd.f32x8 = 1
	i64: #simd[64]i8 = 1
	_ = sums2_i32(i)
	_ = sums4_f32(f8)
	_ = sums4_scalar_f32(f4)
	_ = sums4_i8x64(i64)
}
