package main

import "base:intrinsics"

copy_non_overlapping :: proc(dst, src: ^u32) {
	intrinsics.mem_copy_non_overlapping(dst, src, size_of(u32))
}

unlikely :: proc(value: bool) -> bool {
	return intrinsics.unlikely(value)
}

expect_bool :: proc(value: bool) -> bool {
	return intrinsics.expect(value, false)
}

expect_i32 :: proc(value: i32) -> i32 {
	return intrinsics.expect(value, 0)
}

main :: proc() {
	src: u32 = 0x030201
	dst: u32
	copy_non_overlapping(&dst, &src)
	_ = unlikely(dst != 0)
	_ = expect_bool(dst != 0)
	_ = expect_i32(1)
}
