package main

import "core:simd"

to_bits_f32 :: proc(x: simd.f32x4) -> simd.u32x4 { return simd.to_bits(x) }
to_bits_signed_u32 :: proc(x: simd.u32x4) -> simd.i32x4 { return simd.to_bits_signed(x) }
saturating_add_i8 :: proc(x, y: simd.i8x16) -> simd.i8x16 { return simd.saturating_add(x, y) }
saturating_sub_u8 :: proc(x, y: simd.u8x16) -> simd.u8x16 { return simd.saturating_sub(x, y) }

main :: proc() {
	f: simd.f32x4 = 1
	i: simd.i8x16 = 1
	u: simd.u8x16 = 1
	_ = to_bits_f32(f)
	_ = to_bits_signed_u32(simd.u32x4(1))
	_ = saturating_add_i8(i, i)
	_ = saturating_sub_u8(u, u)
}
