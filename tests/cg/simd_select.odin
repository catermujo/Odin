package main

import "core:simd"

select_u8 :: proc(cond: simd.u8x16, x, y: simd.u8x16) -> simd.u8x16 {
	return simd.select(cond, x, y)
}

select_f32 :: proc(cond: simd.u32x4, x, y: simd.f32x4) -> simd.f32x4 {
	return simd.select(cond, x, y)
}

main :: proc() {
	cond_u8: simd.u8x16 = 1
	x_u8: simd.u8x16 = 2
	y_u8: simd.u8x16 = 3
	cond_f32: simd.u32x4 = 1
	x_f32: simd.f32x4 = 2
	y_f32: simd.f32x4 = 3
	_ = select_u8(cond_u8, x_u8, y_u8)
	_ = select_f32(cond_f32, x_f32, y_f32)
}
