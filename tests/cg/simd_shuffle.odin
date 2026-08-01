package main

import "core:simd"

shuffle_u32 :: proc(x, y: simd.u32x4) -> simd.u32x4 { return simd.shuffle(x, y, 0, 5, 2, 7) }
odd_even_u32 :: proc(x, y: simd.u32x4) -> simd.u32x4 { return simd.odd_even(x, y) }
reverse_u32 :: proc(x: simd.u32x4) -> simd.u32x4 { return simd.lanes_reverse(x) }
rotate_left_u32 :: proc(x: simd.u32x4) -> simd.u32x4 { return simd.lanes_rotate_left(x, 1) }
rotate_right_u32 :: proc(x: simd.u32x4) -> simd.u32x4 { return simd.lanes_rotate_right(x, 1) }
rotate_left_negative_u32 :: proc(x: simd.u32x4) -> simd.u32x4 { return simd.lanes_rotate_left(x, -1) }
rotate_right_negative_u32 :: proc(x: simd.u32x4) -> simd.u32x4 { return simd.lanes_rotate_right(x, -1) }

main :: proc() {
	x: simd.u32x4 = 1
	y: simd.u32x4 = 2
	_ = shuffle_u32(x, y)
	_ = odd_even_u32(x, y)
	_ = reverse_u32(x)
	_ = rotate_left_u32(x)
	_ = rotate_right_u32(x)
	_ = rotate_left_negative_u32(x)
	_ = rotate_right_negative_u32(x)
}
