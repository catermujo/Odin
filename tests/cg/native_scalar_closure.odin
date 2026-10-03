package native_scalar_closure

import "base:intrinsics"

foreign {
	native_scalar_sink :: proc "c" (a: i64, b: u64, c: bool, d: f64) ---
	native_scalar_pair_sink :: proc "c" (a: u32, b: bool) ---
}

Pair :: struct {
	value: u64,
}

@(export)
native_narrow_casts :: proc "c" (a: i8, b: u8, c: i16, d: u16) -> i64 {
	return i64(a) + i64(b) + i64(c) + i64(d)
}

@(export)
native_narrow_locals :: proc "c" (value: i8) -> i8 {
	local := value
	local = i8(local + 3)
	return local
}

@(export)
native_narrow_loop :: proc "c" (value: i8, count: u16) -> i64 {
	total := i64(value)
	i := count
	for i != 0 {
		total += i64(i8(i))
		i -= 1
	}
	return total
}

@(export)
native_narrow_bits :: proc "c" (value: u8, shift: u8) -> u8 {
	return u8(intrinsics.reverse_bits(u16(value)))
}

@(export)
native_narrow_unary :: proc "c" (value: u16) -> u16 {
	return intrinsics.byte_swap(value) ~ intrinsics.count_leading_zeros(value) ~ intrinsics.count_trailing_zeros(value)
}

@(export)
native_narrow_compare :: proc "c" (value: i8, other: i16, pick: bool) -> i16 {
	return i16(value) if pick && value < i8(other) else other
}

@(export)
native_f16_convert :: proc "c" (value: f16) -> f64 {
	return f64(value)
}

@(export)
native_f16_bits :: proc "c" (value: u16) -> u16 {
	return transmute(u16)transmute(f16)value
}

@(export)
native_float_bits :: proc "c" (f16_value: f16, f32_value: f32, f64_value: f64) -> u64 {
	a := u64(transmute(u16)transmute(f16)f16_value)
	b := u64(transmute(u32)transmute(f32)f32_value)
	c := transmute(u64)transmute(f64)f64_value
	return a ~ (b << 16) ~ c
}

@(export)
native_float_abs :: proc "c" (a: f16, b: f32, c: f64) -> f64 {
	return f64(abs(a)) + f64(abs(b)) + abs(c)
}

@(export)
native_scalar_intrinsics :: proc "c" (value: i32, other: u32) -> i64 {
	sat := intrinsics.saturating_add(value, 7)
	fixed := intrinsics.fixed_point_mul(value, 256, 8)
	overflow, _ := intrinsics.overflow_mul(i64(value), i64(other))
	bits := intrinsics.byte_swap(other) ~ intrinsics.reverse_bits(other)
	counts := intrinsics.count_leading_zeros(other) + intrinsics.count_trailing_zeros(other)
	return i64(sat) + i64(fixed) + overflow + i64(bits) + i64(counts)
}

@(export)
native_scalar_saturating :: proc "c" (si8: i8, ui8: u8, si16: i16, ui16: u16, si64: i64, ui64: u64) -> i64 {
	return i64(intrinsics.saturating_add(si8, 3)) + i64(intrinsics.saturating_sub(ui8, 3)) +
		i64(intrinsics.saturating_add(si16, 3)) + i64(intrinsics.saturating_sub(ui16, 3)) +
		si64 + i64(ui64)
}

@(export)
native_scalar_saturating_narrow :: proc "c" (a, b: i8, c, d: u8, e, f: i16, g, h: u16) -> i64 {
	add8 := intrinsics.saturating_add(a, b)
	sub8 := intrinsics.saturating_sub(c, d)
	add16 := intrinsics.saturating_add(e, f)
	sub16 := intrinsics.saturating_sub(g, h)
	return i64(add8) + i64(sub8) + i64(add16) + i64(sub16)
}

@(export)
native_scalar_saturating_wide :: proc "c" (a, b: i32, c, d: u32, e, f: i64, g, h: u64) -> i64 {
	add32 := intrinsics.saturating_add(a, b)
	sub32 := intrinsics.saturating_sub(c, d)
	add64 := intrinsics.saturating_add(e, f)
	sub64 := intrinsics.saturating_sub(g, h)
	return i64(add32) + i64(sub32) + add64 + i64(sub64)
}

@(export)
native_scalar_fixed :: proc "c" (si32, sj32: i32, ui32, uj32: u32) -> i64 {
	return i64(intrinsics.fixed_point_mul(si32, sj32, 8)) + i64(intrinsics.fixed_point_div(si32, sj32, 8)) +
		i64(intrinsics.fixed_point_mul_sat(ui32, uj32, 8)) + i64(intrinsics.fixed_point_div_sat(ui32, uj32, 8))
}

@(export)
native_scalar_fixed_signed_sat :: proc "c" (lhs, rhs: i32) -> i64 {
	return i64(intrinsics.fixed_point_mul_sat(lhs, rhs, 8)) + i64(intrinsics.fixed_point_div_sat(lhs, rhs, 8))
}

@(export)
native_scalar_fixed32_scale_edges :: proc "c" (lhs, rhs: i32, ulhs, urhs: u32) -> i64 {
	signed_low := intrinsics.fixed_point_mul(lhs, rhs, 1)
	signed_high := intrinsics.fixed_point_div(lhs, rhs, 31)
	unsigned_low := intrinsics.fixed_point_mul_sat(ulhs, urhs, 1)
	unsigned_high := intrinsics.fixed_point_div_sat(ulhs, urhs, 31)
	return i64(signed_low) + i64(signed_high) + i64(unsigned_low) + i64(unsigned_high)
}

@(export)
native_scalar_fixed32_signed_mul_scales :: proc "c" (lhs, rhs: i32) -> i64 {
	return i64(intrinsics.fixed_point_mul_sat(lhs, rhs, 1)) +
		i64(intrinsics.fixed_point_mul_sat(lhs, rhs, 8)) +
		i64(intrinsics.fixed_point_mul_sat(lhs, rhs, 31))
}

@(export)
native_scalar_fixed32_signed_div_scales :: proc "c" (lhs, rhs: i32) -> i64 {
	return i64(intrinsics.fixed_point_div_sat(lhs, rhs, 1)) +
		i64(intrinsics.fixed_point_div_sat(lhs, rhs, 8)) +
		i64(intrinsics.fixed_point_div_sat(lhs, rhs, 31))
}

@(export)
native_scalar_fixed32_unsigned_mul_scales :: proc "c" (lhs, rhs: u32) -> i64 {
	return i64(intrinsics.fixed_point_mul_sat(lhs, rhs, 1)) +
		i64(intrinsics.fixed_point_mul_sat(lhs, rhs, 8)) +
		i64(intrinsics.fixed_point_mul_sat(lhs, rhs, 31))
}

@(export)
native_scalar_fixed32_unsigned_div_scales :: proc "c" (lhs, rhs: u32) -> i64 {
	return i64(intrinsics.fixed_point_div_sat(lhs, rhs, 1)) +
		i64(intrinsics.fixed_point_div_sat(lhs, rhs, 8)) +
		i64(intrinsics.fixed_point_div_sat(lhs, rhs, 31))
}

@(export)
native_scalar_fixed64_scale1_mul :: proc "c" (lhs, rhs: i64) -> i64 {
	return intrinsics.fixed_point_mul_sat(lhs, rhs, 1)
}

@(export)
native_scalar_fixed64_scale63_mul :: proc "c" (lhs, rhs: i64) -> i64 {
	return intrinsics.fixed_point_mul_sat(lhs, rhs, 63)
}

@(export)
native_scalar_fixed64_scale1_div :: proc "c" (lhs, rhs: i64) -> i64 {
	return intrinsics.fixed_point_div_sat(lhs, rhs, 1)
}

@(export)
native_scalar_fixed64_scale63_div :: proc "c" (lhs, rhs: i64) -> i64 {
	return intrinsics.fixed_point_div_sat(lhs, rhs, 63)
}

@(export)
native_scalar_overflow :: proc "c" (si32, sj32: i32, ui32, uj32: u32) -> (i64, bool) {
	signed, signed_overflow := intrinsics.overflow_mul(si32, sj32)
	unsigned, unsigned_overflow := intrinsics.overflow_mul(ui32, uj32)
	return i64(signed) + i64(unsigned), signed_overflow || unsigned_overflow
}

@(export)
native_scalar_overflow64 :: proc "c" (si, sj: i64, ui, uj: u64) -> (i64, bool) {
	signed, signed_overflow := intrinsics.overflow_mul(si, sj)
	unsigned, unsigned_overflow := intrinsics.overflow_mul(ui, uj)
	return signed + i64(unsigned), signed_overflow || unsigned_overflow
}

@(export)
native_scalar_fixed64 :: proc "c" (si, sj: i64, ui, uj: u64) -> i64 {
	signed_mul := intrinsics.fixed_point_mul(si, sj, 8)
	signed_div := intrinsics.fixed_point_div(si, sj, 8)
	unsigned_mul := intrinsics.fixed_point_mul_sat(ui, uj, 8)
	unsigned_div := intrinsics.fixed_point_div_sat(ui, uj, 8)
	return signed_mul + signed_div + i64(unsigned_mul) + i64(unsigned_div)
}

@(export)
native_scalar_fixed64_wide :: proc "c" (si, sj: i64, ui, uj: u64) -> i64 {
	return intrinsics.fixed_point_mul(si, sj, 8) + i64(intrinsics.fixed_point_div(ui, uj, 8))
}

@(export)
native_scalar_fixed64_signed_sat :: proc "c" (lhs, rhs: i64) -> i64 {
	return intrinsics.fixed_point_mul_sat(lhs, rhs, 8) + intrinsics.fixed_point_div_sat(lhs, rhs, 8)
}

@(export)
native_scalar_fixed64_signed_mul_scales :: proc "c" (lhs, rhs: i64) -> i64 {
	return intrinsics.fixed_point_mul_sat(lhs, rhs, 1) +
		intrinsics.fixed_point_mul_sat(lhs, rhs, 8) +
		intrinsics.fixed_point_mul_sat(lhs, rhs, 63)
}

@(export)
native_scalar_fixed64_signed_div_scales :: proc "c" (lhs, rhs: i64) -> i64 {
	return intrinsics.fixed_point_div_sat(lhs, rhs, 1) +
		intrinsics.fixed_point_div_sat(lhs, rhs, 8) +
		intrinsics.fixed_point_div_sat(lhs, rhs, 63)
}

@(export)
native_scalar_fixed64_unsigned_mul_scales :: proc "c" (lhs, rhs: u64) -> i64 {
	return i64(intrinsics.fixed_point_mul_sat(lhs, rhs, 1)) +
		i64(intrinsics.fixed_point_mul_sat(lhs, rhs, 8)) +
		i64(intrinsics.fixed_point_mul_sat(lhs, rhs, 63))
}

@(export)
native_scalar_fixed64_unsigned_div_scales :: proc "c" (lhs, rhs: u64) -> i64 {
	return i64(intrinsics.fixed_point_div_sat(lhs, rhs, 1)) +
		i64(intrinsics.fixed_point_div_sat(lhs, rhs, 8)) +
		i64(intrinsics.fixed_point_div_sat(lhs, rhs, 63))
}

@(export)
native_scalar_f16_to_f32 :: proc "c" (half: f16) -> f32 {
	return f32(half)
}

@(export)
native_scalar_f16_to_f64 :: proc "c" (half: f16) -> f64 {
	return f64(half)
}

@(export)
native_scalar_f32_to_f16 :: proc "c" (single: f32) -> f16 {
	return f16(single)
}

@(export)
native_scalar_f64_to_f16 :: proc "c" (double: f64) -> f16 {
	return f16(double)
}

@(export)
native_scalar_i32_to_f16 :: proc "c" (integer: i32) -> f16 {
	return f16(integer)
}

@(export)
native_scalar_f16_to_i32 :: proc "c" (half: f16) -> i32 {
	return i32(half)
}

@(export)
native_scalar_f16_to_u32 :: proc "c" (half: f16) -> u32 {
	return u32(half)
}

@(export)
native_scalar_f16_compare :: proc "c" (left, right: f16, pick: bool) -> f16 {
	return left if pick && left < right else right
}

@(export)
native_scalar_f16_equal :: proc "c" (left, right: f16) -> bool {
	return left == right
}

@(export)
native_scalar_expect :: proc "c" (value: u32) -> u32 {
	return intrinsics.expect(value, 0)
}

@(export)
native_scalar_call_args :: proc "c" (a: i32, b: u32, c: bool, d: f32) {
	native_scalar_sink(i64(a), u64(b), c, f64(d))
}

@(export)
native_scalar_select :: proc "c" (value: i32, pick: bool) -> i16 {
	return i16(value) if pick else -7
}

@(export)
native_scalar_multi :: proc "c" (value: u32) -> (u32, bool) {
	return value + 1, value != 0
}

@(export)
native_scalar_multi_forward :: proc "c" (value: u32) -> (u32, bool) {
	return native_scalar_multi(value)
}

@(export)
native_scalar_multi_consumers :: proc "c" (value: u32) -> u32 {
	a, b := native_scalar_multi(value)
	native_scalar_pair_sink(a, b)
	return a + u32(b)
}

@(export)
native_scalar_zero :: proc "c" () -> struct {} {
	return {}
}

@(export)
native_scalar_debug :: proc "c" () {
	intrinsics.debug_trap()
}

@(export)
native_scalar_debug_branch :: proc "c" (value: bool) -> bool {
	if value {
		intrinsics.debug_trap()
	}
	return value
}

@(export)
native_scalar_debug_loop :: proc "c" (count: u32) -> u32 {
	for i := count; i != 0; i -= 1 {
		if i == 1 {
			intrinsics.debug_trap()
		}
	}
	return count
}

@(export)
native_scalar_prefetch :: proc "c" (address: rawptr) {
	intrinsics.prefetch_read_data(address, 3)
}

@(export)
native_scalar_prefetch_instruction :: proc "c" (address: rawptr) {
	intrinsics.prefetch_read_instruction(address, 0)
}

@(export)
native_scalar_prefetch_write_data :: proc "c" (address: rawptr) {
	intrinsics.prefetch_write_data(address, 2)
}

@(export)
native_scalar_prefetch_write_instruction :: proc "c" (address: rawptr) {
	intrinsics.prefetch_write_instruction(address, 1)
}

main :: proc() {}
