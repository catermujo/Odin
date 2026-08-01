package main

import "core:simd"

extract_u32 :: proc(x: simd.u32x4, index: int) -> u32 { return simd.extract(x, index) }
replace_u32 :: proc(x: simd.u32x4, index: int, value: u32) -> simd.u32x4 { return simd.replace(x, index, value) }

main :: proc() {
	x: simd.u32x4 = 1
	_ = extract_u32(x, 2)
	_ = replace_u32(x, 1, 2)
}
