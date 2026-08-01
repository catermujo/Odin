package main

import "core:simd"

and_u8 :: proc(a: simd.u8x16) -> u8 { return simd.reduce_and(a) }
or_u8  :: proc(a: simd.u8x16) -> u8 { return simd.reduce_or(a) }
xor_u8 :: proc(a: simd.u8x16) -> u8 { return simd.reduce_xor(a) }
min_u8 :: proc(a: simd.u8x16) -> u8 { return simd.reduce_min(a) }
max_u8 :: proc(a: simd.u8x16) -> u8 { return simd.reduce_max(a) }
min_i8 :: proc(a: simd.i8x16) -> i8 { return simd.reduce_min(a) }
max_i8 :: proc(a: simd.i8x16) -> i8 { return simd.reduce_max(a) }
min_f32 :: proc(a: simd.f32x4) -> f32 { return simd.reduce_min(a) }
max_f32 :: proc(a: simd.f32x4) -> f32 { return simd.reduce_max(a) }

main :: proc() {
	u: simd.u8x16 = 1
	i: simd.i8x16 = 1
	f: simd.f32x4 = 1
	_ = and_u8(u)
	_ = or_u8(u)
	_ = xor_u8(u)
	_ = min_u8(u)
	_ = max_u8(u)
	_ = min_i8(i)
	_ = max_i8(i)
	_ = min_f32(f)
	_ = max_f32(f)
}
