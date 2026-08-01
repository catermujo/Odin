package main

import "base:intrinsics"

fixed_point_signed :: proc(lhs, rhs: i32) -> i32 {
	product := intrinsics.fixed_point_mul(lhs, rhs, 8)
	quotient := intrinsics.fixed_point_div(lhs, rhs, 8)
	product_sat := intrinsics.fixed_point_mul_sat(lhs, rhs, 8)
	quotient_sat := intrinsics.fixed_point_div_sat(lhs, rhs, 8)
	return product + quotient + product_sat + quotient_sat
}

fixed_point_unsigned :: proc(lhs, rhs: u32) -> u32 {
	product := intrinsics.fixed_point_mul(lhs, rhs, 8)
	quotient := intrinsics.fixed_point_div(lhs, rhs, 8)
	product_sat := intrinsics.fixed_point_mul_sat(lhs, rhs, 8)
	quotient_sat := intrinsics.fixed_point_div_sat(lhs, rhs, 8)
	return product + quotient + product_sat + quotient_sat
}

main :: proc() {
	_ = fixed_point_signed(256, 128)
	_ = fixed_point_unsigned(256, 128)
}
