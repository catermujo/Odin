package main

import "core:simd"

Bool_Mask :: #simd[4]bool

eq_u8 :: proc(a, b: simd.u8x16) -> simd.u8x16 { return simd.lanes_eq(a, b) }
ne_u8 :: proc(a, b: simd.u8x16) -> simd.u8x16 { return simd.lanes_ne(a, b) }
lt_u8 :: proc(a, b: simd.u8x16) -> simd.u8x16 { return simd.lanes_lt(a, b) }
le_u8 :: proc(a, b: simd.u8x16) -> simd.u8x16 { return simd.lanes_le(a, b) }
gt_u8 :: proc(a, b: simd.u8x16) -> simd.u8x16 { return simd.lanes_gt(a, b) }
ge_u8 :: proc(a, b: simd.u8x16) -> simd.u8x16 { return simd.lanes_ge(a, b) }

eq_f32 :: proc(a, b: simd.f32x4) -> simd.u32x4 { return simd.lanes_eq(a, b) }
ne_f32 :: proc(a, b: simd.f32x4) -> simd.u32x4 { return simd.lanes_ne(a, b) }
lt_f32 :: proc(a, b: simd.f32x4) -> simd.u32x4 { return simd.lanes_lt(a, b) }
le_f32 :: proc(a, b: simd.f32x4) -> simd.u32x4 { return simd.lanes_le(a, b) }
gt_f32 :: proc(a, b: simd.f32x4) -> simd.u32x4 { return simd.lanes_gt(a, b) }
ge_f32 :: proc(a, b: simd.f32x4) -> simd.u32x4 { return simd.lanes_ge(a, b) }

mask_is_false :: proc(a, b: simd.f32x4) -> bool {
	return Bool_Mask(simd.lanes_gt(a, b)) == false
}

mask_any :: proc(a, b: simd.f32x4) -> bool {
	return transmute(bool)simd.extract_msbs(Bool_Mask(simd.lanes_gt(a, b)))
}

splat_u8 :: proc(value: u8) -> simd.u8x16 { return simd.u8x16(value) }
and_u8 :: proc(a, b: simd.u8x16) -> simd.u8x16 { return simd.bit_and(a, b) }
or_u8 :: proc(a, b: simd.u8x16) -> simd.u8x16 { return simd.bit_or(a, b) }
xor_u8 :: proc(a, b: simd.u8x16) -> simd.u8x16 { return simd.bit_xor(a, b) }
and_not_u8 :: proc(a, b: simd.u8x16) -> simd.u8x16 { return simd.bit_and_not(a, b) }

main :: proc() {
	a: simd.u8x16 = 0
	b: simd.u8x16 = 1
	_ = eq_u8(a, b)
	_ = ne_u8(a, b)
	_ = lt_u8(a, b)
	_ = le_u8(a, b)
	_ = gt_u8(a, b)
	_ = ge_u8(a, b)
	fa: simd.f32x4 = 0
	fb: simd.f32x4 = 1
	_ = eq_f32(fa, fb)
	_ = ne_f32(fa, fb)
	_ = lt_f32(fa, fb)
	_ = le_f32(fa, fb)
	_ = gt_f32(fa, fb)
	_ = ge_f32(fa, fb)
	assert(mask_is_false(fa, fb))
	assert(!mask_is_false(fb, fa))
	assert(!mask_any(fa, fb))
	assert(mask_any(fb, fa))
	_ = splat_u8(1)
	_ = and_u8(a, b)
	_ = or_u8(a, b)
	_ = xor_u8(a, b)
	_ = and_not_u8(a, b)
}
