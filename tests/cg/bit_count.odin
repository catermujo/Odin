package main

import "base:intrinsics"

count_bits :: proc(value: u64) -> u64 {
	return intrinsics.count_ones(value) +
	       intrinsics.count_zeros(value) +
	       intrinsics.count_trailing_zeros(value) +
	       intrinsics.count_leading_zeros(value) +
	       intrinsics.count_trailing_ones(value) +
	       intrinsics.count_leading_ones(value)
}

main :: proc() {
	_ = count_bits(0x0010_ffff_0000_000f)
}
