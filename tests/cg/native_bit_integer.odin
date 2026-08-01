package native_bit_integer

import "base:intrinsics"

foreign {
	native_bit_integer_sink :: proc "c" (lhs, rhs: u64) ---
}

@(export)
native_m12_u32 :: proc "c" (lhs, rhs, shift: u32) -> u32 {
	return (lhs << shift) ~ (rhs >> shift) ~ (lhs / rhs) ~ (lhs % rhs)
}

@(export)
native_m12_i32 :: proc "c" (lhs, rhs: i32, shift: u32) -> i32 {
	return (lhs << shift) + (rhs >> shift) + (lhs / rhs) + (lhs % rhs)
}

@(export)
native_m12_u64 :: proc "c" (lhs, rhs, shift: u64) -> u64 {
	return (lhs << shift) ~ (rhs >> shift) ~ (lhs / rhs) ~ (lhs % rhs)
}

@(export)
native_m12_i64 :: proc "c" (lhs, rhs: i64, shift: u64) -> i64 {
	return (lhs << shift) + (rhs >> shift) + (lhs / rhs) + (lhs % rhs)
}

@(export)
native_m12_u32_ops_in_i64 :: proc "c" (lhs, rhs: u32) -> i64 {
	return i64(lhs / rhs) + i64(lhs % rhs)
}

@(export)
native_m12_i32_ops_in_u64 :: proc "c" (lhs, rhs: i32) -> u64 {
	return u64(lhs / rhs) + u64(lhs % rhs)
}

@(export)
native_m12_divzero_u32 :: proc "c" (divisor: u32) -> u32 {
	return 7 / divisor
}

@(export)
native_m12_divzero_i64 :: proc "c" (divisor: i64) -> i64 {
	return 7 / divisor
}

@(export)
native_m12_modzero_u32 :: proc "c" (divisor: u32) -> u32 {
	return 7 % divisor
}

@(export)
native_m12_modzero_i64 :: proc "c" (divisor: i64) -> i64 {
	return 7 % divisor
}

@(export)
native_m12_unsigned_literal_right_div :: proc "c" (divisor: u32) -> u32 {
	return divisor / 7
}

@(export)
native_m12_unsigned_literal_right_mod :: proc "c" (divisor: u32) -> u32 {
	return divisor % 7
}

@(export)
native_m12_signed_literal_left_div :: proc "c" (divisor: i32) -> i32 {
	return 7 / divisor
}

@(export)
native_m12_signed_literal_right_div :: proc "c" (divisor: i32) -> i32 {
	return divisor / 7
}

@(export)
native_m12_signed_literal_left_mod :: proc "c" (divisor: i32) -> i32 {
	return 7 % divisor
}

@(export)
native_m12_signed_literal_right_mod :: proc "c" (divisor: i32) -> i32 {
	return divisor % 7
}

@(export)
native_m12_unsigned_local_div_mod :: proc "c" (divisor: u32) -> u32 {
	local := divisor
	return (7 / local) + (local % 7)
}

@(export)
native_m12_signed_local_div_mod :: proc "c" (divisor: i32) -> i32 {
	local := divisor
	return (7 / local) + (local % 7)
}

@(export)
native_m12_rol_u32 :: proc "c" (value, shift: u32) -> u32 {
	return (value << (shift & 31)) | (value >> (32-(shift & 31)))
}

@(export)
native_m12_ror_u32 :: proc "c" (value, shift: u32) -> u32 {
	return (value >> (shift & 31)) | (value << (32-(shift & 31)))
}

@(export)
native_m12_rol_u64 :: proc "c" (value, shift: u64) -> u64 {
	return (value << (shift & 63)) | (value >> (64-(shift & 63)))
}

@(export)
native_m12_ror_u64 :: proc "c" (value, shift: u64) -> u64 {
	return (value >> (shift & 63)) | (value << (64-(shift & 63)))
}

@(export)
native_m12_unary_u32 :: proc "c" (value: u32) -> u32 {
	return intrinsics.byte_swap(value) ~ intrinsics.reverse_bits(value) ~ intrinsics.count_leading_zeros(value) ~ intrinsics.count_trailing_zeros(value)
}

@(export)
native_m12_unary_u64 :: proc "c" (value: u64) -> u64 {
	return intrinsics.byte_swap(value) ~ intrinsics.reverse_bits(value) ~ intrinsics.count_leading_zeros(value) ~ intrinsics.count_trailing_zeros(value)
}

@(export)
native_m12_global: u64

@(export)
native_m12_global_store :: proc "c" (value: u64) {
	native_m12_global = intrinsics.byte_swap(value)
}

native_m12_internal :: proc "c" (value: u64) -> u64 {
	return intrinsics.reverse_bits(value) / 3
}

@(export)
native_m12_call_args :: proc "c" (lhs, rhs: u64) {
	native_bit_integer_sink(lhs << 3, intrinsics.byte_swap(rhs))
}

@(export)
native_m12_call_return :: proc "c" (value: u64) -> u64 {
	return native_m12_internal(value / 3)
}

@(export)
native_m12_loop :: proc "c" (value, limit: u64) -> u64 {
	total := value
	i := limit
	for i != 0 {
		total = total + intrinsics.byte_swap(total) ~ (total / (i | 1))
		i -= 1
	}
	return total
}

main :: proc() {}
