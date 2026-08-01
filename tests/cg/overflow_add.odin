package main

import "base:intrinsics"

overflow_add :: proc(x, y: u64) -> (u64, bool) {
	return intrinsics.overflow_add(x, y)
}

overflow_add_signed :: proc(x, y: i64) -> (i64, bool) {
	return intrinsics.overflow_add(x, y)
}

main :: proc() {
	_, _ = overflow_add(1, 2)
	_, _ = overflow_add_signed(1, 2)
}
