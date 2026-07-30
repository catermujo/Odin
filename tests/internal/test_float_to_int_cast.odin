package test_internal

import "core:testing"

cast_f32_to_i64 :: #force_no_inline proc(v: f32) -> i64 {
	return i64(v)
}

cast_f32_to_u64 :: #force_no_inline proc(v: f32) -> u64 {
	return u64(v)
}

@test
test_float_to_int_64_cast :: proc(t: ^testing.T) {
	testing.expect_value(t, cast_f32_to_i64(f32(2147483648)), i64(2147483648))
	testing.expect_value(t, cast_f32_to_u64(f32(4294967296)), u64(4294967296))
}
