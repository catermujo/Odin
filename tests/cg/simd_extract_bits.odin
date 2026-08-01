package main

import "core:simd"

Bits16 :: bit_set[0..<16]
Bits64 :: bit_set[0..<64]

lsbs_u8 :: proc(x: simd.u8x16) -> Bits16 { return simd.extract_lsbs(x) }
msbs_i8 :: proc(x: simd.i8x16) -> Bits16 { return simd.extract_msbs(x) }
lsbs_u8x64 :: proc(x: #simd[64]u8) -> Bits64 { return simd.extract_lsbs(x) }
msbs_i8x64 :: proc(x: #simd[64]i8) -> Bits64 { return simd.extract_msbs(x) }

main :: proc() {
	a: simd.u8x16 = 1
	b: simd.i8x16 = -1
	c: #simd[64]u8 = 1
	d: #simd[64]i8 = -1
	_ = lsbs_u8(a)
	_ = msbs_i8(b)
	_ = lsbs_u8x64(c)
	_ = msbs_i8x64(d)
}
