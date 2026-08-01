package main

import "base:intrinsics"

U128_LOW_HIGH :: (u128(0x1122334455667788) << 64) | u128(0x99aabbccddeeff01)
U128_HIGH_ONLY :: u128(0xfedcba9876543210) << 64
U128_TOP_BIT :: u128(1) << 127
U128_ALL_BITS :: ~u128(0)
I128_POSITIVE :: i128((u128(0x123456789abcdef0) << 64) | u128(0x0fedcba987654321))
I128_NEGATIVE :: -i128((u128(0x13579bdf2468ace0) << 64) | u128(0xfdb97531eca86421))

Bits128 :: bit_set[0..<128; u128]
Sparse_Flag :: enum {
	Low  = 0,
	Mid  = 64,
	High = 100,
	Top  = 127,
}
Sparse_Bits128 :: bit_set[Sparse_Flag; u128]

Packed_I128 :: struct #packed {
	tag:   u8,
	value: u128,
}

Aligned8_I128 :: struct #align(8) {
	value: u128,
}

I128_Result :: struct {
	tag:    u64,
	first:  u128,
	second: i128,
	tail:   u64,
}

Nested_I128_Result :: struct {
	prefix: [3]u64,
	inner:  I128_Result,
	values: [2]u128,
	suffix: u32,
}

@(export)
native_i128_writable_global: u128 = U128_LOW_HIGH

@(export)
native_i128_signed_global: i128 = I128_NEGATIVE

@(export)
native_i128_array_global: [3]u128 = {
	U128_LOW_HIGH,
	U128_HIGH_ONLY | 0x0123456789abcdef,
	U128_TOP_BIT | 0x0f1e2d3c4b5a6978,
}

@(rodata)
native_i128_readonly_global: u128 = U128_TOP_BIT | 0x76543210fedcba98

@(rodata)
native_i128_bitset_global: Bits128 = {0, 63, 64, 100, 127}

foreign {
	native_i128_foreign :: proc "c" (value: u128) -> u128 ---
}

native_i128_contextless_callee :: proc "contextless" (value: i128) -> i128 {
	return value + I128_POSITIVE
}

native_i128_odin_callee :: proc "odin" (value: i128) -> i128 {
	return value + i128(context.user_index) + I128_POSITIVE
}

native_i128_odin_exhausted_callee :: proc "odin" (
	a0, a1, a2, a3, a4, a5: u64,
	value: u128,
) -> u128 {
	return value + u128(a0+a1+a2+a3+a4+a5+u64(context.user_index))
}

native_i128_c_callee :: proc "c" (value: u128) -> u128 {
	return value ~ U128_LOW_HIGH
}

native_i128_pair_callee :: proc "c" (lhs, rhs: u128) -> u128 {
	return lhs + rhs
}

native_i128_scalar_helper :: proc "contextless" (value: u64) -> u64 {
	return value ~ 0x5a5aa5a5f0f00f0f
}

native_i128_apply_closure :: proc(call: lambda(u128) -> u128, value: u128) -> u128 {
	return call(value)
}

native_i128_apply_exhausted_closure :: proc(
	call: lambda(u64, u64, u64, u64, u64, u64, u128) -> u128,
	a0, a1, a2, a3, a4, a5: u64,
	value: u128,
) -> u128 {
	return call(a0, a1, a2, a3, a4, a5, value)
}

@(export)
native_i128_constants :: proc "c" (selector: u64) -> u128 {
	switch selector {
	case 0: return 0
	case 1: return 1
	case 2: return U128_ALL_BITS
	case 3: return U128_HIGH_ONLY
	case 4: return U128_TOP_BIT
	case 5: return transmute(u128)min(i128)
	case 6: return u128(max(i128))
	case 7: return transmute(u128)I128_NEGATIVE
	case:   return U128_LOW_HIGH
	}
}

@(export)
native_i128_cast_signed :: proc "c" (s8: i8, s16: i16, s32: i32, s64: i64) -> i128 {
	return i128(s8) ~ i128(s16) ~ i128(s32) ~ i128(s64)
}

@(export)
native_i128_cast_unsigned :: proc "c" (
	flag: bool,
	u8_value: u8,
	u16_value: u16,
	u32_value: u32,
	u64_value: u64,
) -> u128 {
	return u128(flag) | u128(u8_value) | u128(u16_value) | u128(u32_value) | u128(u64_value)
}

@(export)
native_i128_truncate :: proc "c" (value: u128) -> (bool, i8, u16, i32, u64) {
	return bool(value), i8(value), u16(value), i32(value), u64(value)
}

@(export)
native_i128_representation_casts :: proc "c" (signed: i128, unsigned: u128) -> u128 {
	return u128(signed) ~ u128(i128(unsigned))
}

@(export)
native_i128_endian_casts :: proc "c" (value: u128) -> u128 {
	little := u128le(value)
	big := u128be(value)
	return u128(little) ~ u128(big)
}

@(export)
native_i128_local_memory :: proc "c" (value: u128) -> u128 {
	local := value
	copy := local
	return copy
}

@(export)
native_i128_pointer_memory :: proc "c" (destination, source: ^u128) -> u128 {
	value := source^
	destination^ = value
	return destination^
}

@(export)
native_i128_global_memory :: proc "c" (value: u128, index: int) -> u128 {
	native_i128_writable_global = value
	native_i128_signed_global = i128(value ~ U128_LOW_HIGH)
	native_i128_array_global[index] = native_i128_writable_global
	return native_i128_array_global[index] ~ u128(native_i128_signed_global) ~ native_i128_readonly_global
}

@(export)
native_i128_packed_memory :: proc "c" (destination, source: ^Packed_I128) -> u128 {
	value := source.value
	destination.value = value
	return destination.value
}

@(export)
native_i128_aligned8_memory :: proc "c" (destination, source: ^Aligned8_I128) -> u128 {
	value := source.value
	destination.value = value
	return destination.value
}

@(export)
native_i128_unaligned_memory :: proc "c" (base: ^u8, value: u128) -> u128 {
	offset1 := intrinsics.ptr_offset(base, 1)
	offset8 := intrinsics.ptr_offset(base, 8)
	intrinsics.unaligned_store((^u128)(offset1), value)
	first := intrinsics.unaligned_load((^u128)(offset1))
	intrinsics.unaligned_store((^u128)(offset8), first ~ U128_LOW_HIGH)
	return intrinsics.unaligned_load((^u128)(offset8))
}

@(export)
native_i128_volatile_memory :: proc "c" (source, destination: ^u128) -> u128 {
	value := intrinsics.volatile_load(source)
	intrinsics.volatile_store(destination, value)
	return value + value
}

@(export)
native_i128_volatile_call_survival :: proc "c" (source: ^u128, seed: u64) -> u128 {
	value := intrinsics.volatile_load(source)
	scalar := native_i128_scalar_helper(seed)
	return value + value + u128(scalar)
}

@(export)
native_i128_array_field_memory :: proc "c" (result: ^I128_Result, values: ^[3]u128, index: int) -> u128 {
	value := values[index]
	result.first = value
	result.second = i128(values[(index+1)%3])
	return result.first ~ u128(result.second)
}

@(export)
native_i128_arithmetic :: proc "c" (lhs, rhs: u128) -> u128 {
	added := lhs + rhs
	subtracted := lhs - rhs
	negated := -lhs
	product := lhs * rhs
	return added ~ subtracted ~ negated ~ product
}

@(export)
native_i128_bitwise :: proc "c" (lhs, rhs: u128) -> u128 {
	return (lhs & rhs) ~ (lhs | rhs) ~ (lhs ~ rhs) ~ (lhs &~ rhs) ~ ~lhs
}

@(export)
native_i128_unary_bits :: proc "c" (value: u128) -> u128 {
	swapped := intrinsics.byte_swap(value)
	reversed := intrinsics.reverse_bits(value)
	leading := intrinsics.count_leading_zeros(value)
	trailing := intrinsics.count_trailing_zeros(value)
	ones := intrinsics.count_ones(value)
	return swapped ~ reversed ~ leading ~ trailing ~ ones
}

@(export)
native_i128_shift_dynamic_unsigned :: proc "c" (value, count: u128) -> u128 {
	return (value << count) ~ (value >> count)
}

@(export)
native_i128_shift_dynamic_signed :: proc "c" (value: i128, count: u128) -> i128 {
	return (value << count) ~ (value >> count)
}

@(export)
native_i128_shift_nested_scalar_unsigned :: proc "c" (
	value: u128,
	count, factor, bias: u64,
	choose_xor: bool,
) -> u128 {
	left := value << (count*factor+bias)
	scalar := native_i128_scalar_helper(bias)
	right := value >> (count*(factor|1)+bias+(scalar&3))
	if choose_xor {
		return left ~ right ~ u128(scalar)
	}
	return left + right + (u128(scalar)<<64)
}

@(export)
native_i128_shift_nested_scalar_signed :: proc "c" (
	value: i128,
	count, factor, bias: u64,
	choose_xor: bool,
) -> i128 {
	left := value << (count*factor+bias)
	scalar := native_i128_scalar_helper(bias)
	right := value >> (count*(factor|1)+bias+(scalar&3))
	if choose_xor {
		return left ~ right ~ i128(scalar)
	}
	return left + right + (i128(scalar)<<64)
}

@(export)
native_i128_shift_boundaries :: proc "c" (value: u128) -> u128 {
	return (value << 0) ~ (value << 1) ~ (value << 63) ~ (value << 64) ~ (value << 65) ~
	       (value << 127) ~ (value << 128) ~ (value << 129) ~ (value >> 0) ~ (value >> 1) ~
	       (value >> 63) ~ (value >> 64) ~ (value >> 65) ~ (value >> 127) ~ (value >> 128) ~
	       (value >> 129)
}

@(export)
native_i128_ashr_boundaries :: proc "c" (value: i128) -> i128 {
	return (value >> 0) ~ (value >> 1) ~ (value >> 63) ~ (value >> 64) ~ (value >> 65) ~
	       (value >> 127) ~ (value >> 128) ~ (value >> 129)
}

native_i128_rotate_left :: proc "contextless" (value, count: u128) -> u128 {
	shift := count & 127
	return (value << shift) | (value >> ((128-shift)&127))
}

native_i128_rotate_right :: proc "contextless" (value, count: u128) -> u128 {
	shift := count & 127
	return (value >> shift) | (value << ((128-shift)&127))
}

@(export)
native_i128_rotate_dynamic :: proc "c" (value, count: u128) -> u128 {
	return native_i128_rotate_left(value, count) ~ native_i128_rotate_right(value, count)
}

@(export)
native_i128_rotate_nested_scalar_unsigned :: proc "c" (
	value: u128,
	count, factor, bias: u64,
	choose_xor: bool,
) -> u128 {
	rotated_left := (value << ((count*factor+bias)&127)) |
	                (value >> ((128-((count*factor+bias)&127))&127))
	scalar := native_i128_scalar_helper(bias)
	rotated_right := (value >> ((count*(factor|1)+bias+(scalar&3))&127)) |
	                 (value << ((128-((count*(factor|1)+bias+(scalar&3))&127))&127))
	if choose_xor {
		return rotated_left ~ rotated_right ~ u128(scalar)
	}
	return rotated_left + rotated_right + (u128(scalar)<<64)
}

@(export)
native_i128_rotate_nested_scalar_signed :: proc "c" (
	value: i128,
	count, factor, bias: u64,
	choose_xor: bool,
) -> i128 {
	bits := u128(value)
	rotated_left := (bits << ((count*factor+bias)&127)) |
	                (bits >> ((128-((count*factor+bias)&127))&127))
	scalar := native_i128_scalar_helper(bias)
	rotated_right := (bits >> ((count*(factor|1)+bias+(scalar&3))&127)) |
	                 (bits << ((128-((count*(factor|1)+bias+(scalar&3))&127))&127))
	if choose_xor {
		return i128(rotated_left ~ rotated_right ~ u128(scalar))
	}
	return i128(rotated_left + rotated_right + (u128(scalar)<<64))
}

@(export)
native_i128_rol_const63 :: proc "c" (value: u128) -> u128 {
	return (value << 63) | (value >> 65)
}

@(export)
native_i128_rol_const64 :: proc "c" (value: u128) -> u128 {
	return (value << 64) | (value >> 64)
}

@(export)
native_i128_rol_const65 :: proc "c" (value: u128) -> u128 {
	return (value << 65) | (value >> 63)
}

@(export)
native_i128_ror_const63 :: proc "c" (value: u128) -> u128 {
	return (value >> 63) | (value << 65)
}

@(export)
native_i128_ror_const64 :: proc "c" (value: u128) -> u128 {
	return (value >> 64) | (value << 64)
}

@(export)
native_i128_ror_const65 :: proc "c" (value: u128) -> u128 {
	return (value >> 65) | (value << 63)
}

@(export)
native_i128_rotate_boundaries :: proc "c" (value: u128) -> u128 {
	return native_i128_rotate_left(value, 0) ~ native_i128_rotate_left(value, 1) ~
	       native_i128_rotate_left(value, 63) ~ native_i128_rotate_left(value, 64) ~
	       native_i128_rotate_left(value, 65) ~ native_i128_rotate_left(value, 127) ~
	       native_i128_rotate_left(value, 128) ~ native_i128_rotate_left(value, 129) ~
	       native_i128_rotate_right(value, 0) ~ native_i128_rotate_right(value, 1) ~
	       native_i128_rotate_right(value, 63) ~ native_i128_rotate_right(value, 64) ~
	       native_i128_rotate_right(value, 65) ~ native_i128_rotate_right(value, 127) ~
	       native_i128_rotate_right(value, 128) ~ native_i128_rotate_right(value, 129)
}

@(export)
native_i128_compare_unsigned :: proc "c" (lhs, rhs: u128) -> u64 {
	result: u64
	if lhs == rhs { result |= 1 }
	if lhs != rhs { result |= 2 }
	if lhs < rhs { result |= 4 }
	if lhs <= rhs { result |= 8 }
	if lhs > rhs { result |= 16 }
	if lhs >= rhs { result |= 32 }
	return result
}

@(export)
native_i128_compare_signed :: proc "c" (lhs, rhs: i128) -> u64 {
	result: u64
	if lhs == rhs { result |= 1 }
	if lhs != rhs { result |= 2 }
	if lhs < rhs { result |= 4 }
	if lhs <= rhs { result |= 8 }
	if lhs > rhs { result |= 16 }
	if lhs >= rhs { result |= 32 }
	return result
}

@(export)
native_i128_select :: proc "c" (condition: bool, lhs, rhs: u128) -> u128 {
	return condition ? lhs : rhs
}

@(export)
native_i128_bitset_membership :: proc "c" (value: Bits128, index: int) -> u64 {
	result: u64
	if 0 in value { result |= 1 }
	if 63 in value { result |= 2 }
	if 64 in value { result |= 4 }
	if 100 in value { result |= 8 }
	if 127 in value { result |= 16 }
	if index in value { result |= 32 }
	if 64 in native_i128_bitset_global { result |= 64 }
	return result
}

@(export)
native_i128_bitset_operations :: proc "c" (lhs, rhs: Bits128) -> Bits128 {
	inserted := lhs + Bits128{0, 64, 127}
	removed := inserted - Bits128{0, 100}
	toggled := removed ~ Bits128{63, 64}
	united := toggled | rhs
	intersection := united & Bits128{0, 63, 64, 100, 127}
	difference := intersection &~ rhs
	return difference | ~intersection
}

@(export)
native_i128_bitset_compare :: proc "c" (lhs, rhs: Bits128) -> u64 {
	result: u64
	if lhs == rhs { result |= 1 }
	if lhs != rhs { result |= 2 }
	return result
}

@(export)
native_i128_bitset_iteration :: proc "c" (value: Bits128, sparse: Sparse_Bits128) -> u128 {
	total: u128
	for index in value {
		total += u128(index)
	}
	for flag in sparse {
		total += u128(flag)
	}
	return total
}

@(export)
native_i128_switch_unsigned :: proc "c" (value: u128) -> u128 {
	switch value {
	case 8:                                      return U128_LOW_HIGH
	case (u128(1)<<96)+8:                        return U128_HIGH_ONLY | 0x0102030405060708
	case U128_TOP_BIT|0x17:                      return U128_TOP_BIT | 0x1122
	case (u128(0x1234)<<64)|0x08:                return U128_LOW_HIGH ~ U128_HIGH_ONLY
	case:                                        return U128_ALL_BITS
	}
}

@(export)
native_i128_switch_signed :: proc "c" (value: i128) -> i128 {
	switch value {
	case -8:                                     return I128_NEGATIVE
	case (i128(1)<<96)+8:                        return I128_POSITIVE
	case -(i128(1)<<100)+8:                      return min(i128)
	case min(i128)+0x19:                          return max(i128)
	case:                                        return -I128_POSITIVE
	}
}

@(export)
native_i128_c_first :: proc "c" (value: u128) -> u128 {
	return native_i128_c_callee(value)
}

@(export)
native_i128_c_odd :: proc "c" (prefix: u64, value: u128, tail: u64) -> u128 {
	return value + u128(prefix) + (u128(tail)<<64)
}

@(export)
native_i128_c_two_gprs :: proc "c" (a0, a1, a2, a3, a4, a5: u64, value: u128, tail: u64) -> u128 {
	return value + u128(a0+a1+a2+a3+a4+a5+tail)
}

@(export)
native_i128_c_stacked :: proc "c" (a0, a1, a2, a3, a4, a5, a6: u64, value: u128, tail: u64) -> u128 {
	return value + u128(a0+a1+a2+a3+a4+a5+a6+tail)
}

@(export)
native_i128_c_fpr_independent :: proc "c" (
	f0, f1, f2, f3, f4, f5, f6, f7, f8: f64,
	a0, a1, a2, a3, a4, a5: u64,
	value: u128,
	tail: u64,
) -> u128 {
	float_tag := u64(0x1020304050607080)
	if f0+f1+f2+f3+f4+f5+f6+f7+f8 < 0 {
		float_tag = 0x8070605040302010
	}
	return value + u128(a0+a1+a2+a3+a4+a5+tail+float_tag)
}

@(export)
native_i128_c_stacked_after_fpr :: proc "c" (
	f0, f1, f2, f3, f4, f5, f6, f7, f8: f64,
	a0, a1, a2, a3, a4, a5, a6: u64,
	value: u128,
	tail: u64,
) -> u128 {
	float_tag := u64(0x1123581321345589)
	if f0+f1+f2+f3+f4+f5+f6+f7+f8 < 0 {
		float_tag = 0x9855342113582311
	}
	return value + u128(a0+a1+a2+a3+a4+a5+a6+tail+float_tag)
}

@(export)
native_i128_multiple_pairs :: proc "c" (first, second, third: u128, tail: u64) -> u128 {
	return first + second + third + u128(tail)
}

@(export)
native_i128_direct_call :: proc "c" (lhs, rhs: u128) -> u128 {
	first := native_i128_c_callee(lhs)
	return native_i128_pair_callee(first, rhs)
}

@(export)
native_i128_odd_call :: proc "c" (value: u128, prefix, tail: u64) -> u128 {
	return native_i128_c_odd(prefix, value, tail)
}

@(export)
native_i128_two_gprs_call :: proc "c" (value: u128) -> u128 {
	return native_i128_c_two_gprs(0x10, 0x21, 0x32, 0x43, 0x54, 0x65, value, 0x76)
}

@(export)
native_i128_stacked_call :: proc "c" (value: u128) -> u128 {
	return native_i128_c_stacked(0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, value, 0x87)
}

@(export)
native_i128_multiple_pairs_call :: proc "c" (first, second, third: u128, tail: u64) -> u128 {
	return native_i128_multiple_pairs(first, second, third, tail)
}

@(export)
native_i128_stacked_after_fpr_call :: proc "c" (value: u128) -> u128 {
	return native_i128_c_stacked_after_fpr(
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8,
		9,
		0x10,
		0x21,
		0x32,
		0x43,
		0x54,
		0x65,
		0x76,
		value,
		0x87,
	)
}

@(export)
native_i128_indirect_call :: proc "c" (value: u128) -> u128 {
	call := native_i128_c_callee
	return call(value)
}

@(export)
native_i128_foreign_call :: proc "c" (value: u128) -> u128 {
	return native_i128_foreign(value)
}

@(export)
native_i128_context_calls :: proc(value: i128) -> i128 {
	first := native_i128_contextless_callee(value)
	return native_i128_odin_callee(first)
}

@(export)
native_i128_odin_exhausted_call :: proc(value: u128) -> u128 {
	return native_i128_odin_exhausted_callee(0x18, 0x29, 0x3a, 0x4b, 0x5c, 0x6d, value)
}

@(export)
native_i128_closure_call :: proc(value: u128) -> u128 {
	bias := U128_HIGH_ONLY | 0x2468ace013579bdf
	add := lambda [bias](input: u128) -> u128 {
		return input + bias + u128(context.user_index)
	}
	return native_i128_apply_closure(add, value)
}

@(export)
native_i128_closure_exhausted_call :: proc(value: u128) -> u128 {
	bias := U128_HIGH_ONLY | 0x3141592653589793
	add := lambda [bias](a0, a1, a2, a3, a4, a5: u64, input: u128) -> u128 {
		return input + bias + u128(a0+a1+a2+a3+a4+a5+u64(context.user_index))
	}
	return native_i128_apply_exhausted_closure(add, 0x17, 0x28, 0x39, 0x4a, 0x5b, 0x6c, value)
}

@(export)
native_i128_sret :: proc "c" (first: u128, second: i128, tag, tail: u64) -> I128_Result {
	return I128_Result{tag = tag, first = first, second = second, tail = tail}
}

@(export)
native_i128_sret_call :: proc "c" (first: u128, second: i128, tag, tail: u64) -> I128_Result {
	return native_i128_sret(first, second, tag, tail)
}

@(export)
native_i128_multi_return :: proc "c" (first, second: u128, tail: u64) -> (u128, u64, u128) {
	return first, tail, second
}

@(export)
native_i128_multi_return_consume :: proc "c" (first, second: u128, tail: u64, choose_xor: bool) -> u128 {
	projected_first, projected_tail, projected_second := native_i128_multi_return(first, second, tail)
	result := projected_first + projected_second + u128(projected_tail)
	if choose_xor {
		result ~= projected_first
	} else {
		result += projected_second
	}
	return result + projected_first + projected_second
}

@(export)
native_i128_nested_sret :: proc "c" (first: u128, second: i128, tag, tail: u64) -> Nested_I128_Result {
	return Nested_I128_Result {
		prefix = {tag ~ 0x1122334455667788, tail ~ 0x8877665544332211, tag + tail},
		inner = {tag = tag, first = first, second = second, tail = tail},
		values = {first ~ U128_LOW_HIGH, u128(second) + U128_HIGH_ONLY},
		suffix = u32(tag ~ tail),
	}
}

@(export)
native_i128_nested_sret_consume :: proc "c" (
	first: u128,
	second: i128,
	tag, tail: u64,
	choose_xor: bool,
) -> u128 {
	result := native_i128_nested_sret(first, second, tag, tail)
	value := result.inner.first + u128(result.inner.second) + result.values[0] + result.values[1]
	if choose_xor {
		value ~= u128(result.prefix[0]) | (u128(result.prefix[1])<<64)
	} else {
		value += u128(result.prefix[2]) + u128(result.suffix)
	}
	return value + result.inner.first + u128(result.inner.second)
}

@(export)
native_i128_dynamic_stacked :: proc "c" (
	a0, a1, a2, a3, a4, a5, a6: u64,
	value: u128,
	tail: u64,
	size: int,
) -> u128 {
	data := intrinsics.alloca(size+1, 32)[:size+1]
	data[0] = u8(a0)
	return value + u128(a0+a1+a2+a3+a4+a5+a6+tail+u64(data[0]))
}

@(export)
native_i128_branch_phi :: proc "c" (condition: bool, first, second: u128) -> u128 {
	result := first
	if condition {
		result = second
	}
	return result + result
}

@(export)
native_i128_loop_phi :: proc "c" (seed, step: u128, count: u64) -> u128 {
	result := seed
	for index := u64(0); index < count; index += 1 {
		result = (result + step) ~ (u128(index)<<64)
	}
	return result
}

@(export)
native_i128_parallel_pair_phis :: proc "c" (
	first, second, third, fourth: u128,
	count: u64,
	rotate_left: bool,
) -> u128 {
	a, b, c, d := first, second, third, fourth
	if rotate_left {
		a, b, c, d = b, c, d, a
	} else {
		a, b, c, d = d, a, b, c
	}
	for index := u64(0); index < count; index += 1 {
		a, b, c, d = b+u128(index), c~(u128(index)<<64), d-u128(index+1), a*3
	}
	return a + (b<<1) ~ (c>>3) + d*U128_LOW_HIGH
}

@(export)
native_i128_parallel_pair_phis_large_frame :: proc "c" (
	first, second, third, fourth: u128,
	count: u64,
	index: int,
	rotate_left: bool,
) -> u128 {
	scratch: [80*1024]u8
	first_slot := index & (len(scratch)-1)
	second_slot := (index+8191) & (len(scratch)-1)
	scratch[first_slot] = u8(first)
	scratch[second_slot] = u8(fourth)
	a, b, c, d := first, second, third, fourth
	if rotate_left {
		a, b, c, d = b, c, d, a
	} else {
		a, b, c, d = d, a, b, c
	}
	for loop_index := u64(0); loop_index < count; loop_index += 1 {
		a, b, c, d = b+u128(loop_index), c~(u128(loop_index)<<64), d-u128(loop_index+1), a*5
	}
	result := a + (b<<1) ~ (c>>3) + d*U128_HIGH_ONLY
	result += u128(scratch[first_slot])
	result ~= u128(scratch[second_slot]) << 64
	return result
}

@(export)
native_i128_duplicate_use_call :: proc "c" (value: u128, seed: u64, choose_xor: bool) -> u128 {
	scalar := native_i128_scalar_helper(seed)
	result := value + value + u128(scalar)
	if choose_xor {
		result ~= value
	} else {
		result += value
	}
	return result
}

@(export)
native_i128_call_result_survival :: proc "c" (value: u128, seed: u64, choose_xor: bool) -> u128 {
	call_result := native_i128_c_callee(value)
	scalar := native_i128_scalar_helper(seed)
	result := call_result + call_result + u128(scalar)
	if choose_xor {
		result ~= call_result
	} else {
		result += call_result
	}
	return result
}

@(export)
native_i128_unsigned_div_rem :: proc "c" (lhs, rhs: u128) -> u128 {
	return (lhs / rhs) ~ (lhs % rhs)
}

@(export)
native_i128_signed_div_rem :: proc "c" (lhs, rhs: i128) -> i128 {
	return (lhs / rhs) ~ (lhs % rhs)
}

@(export)
native_i128_unsigned_div_zero_shape :: proc "c" (divisor: u128) -> u128 {
	return U128_LOW_HIGH / divisor
}

@(export)
native_i128_unsigned_rem_zero_shape :: proc "c" (divisor: u128) -> u128 {
	return U128_HIGH_ONLY % divisor
}

@(export)
native_i128_signed_div_zero_shape :: proc "c" (divisor: i128) -> i128 {
	return I128_NEGATIVE / divisor
}

@(export)
native_i128_signed_rem_zero_shape :: proc "c" (divisor: i128) -> i128 {
	return I128_NEGATIVE % divisor
}

@(export)
native_i128_signed_min_div_minus_one :: proc "c" (divisor: i128) -> i128 {
	return min(i128) / divisor
}

@(export)
native_i128_signed_min_rem_minus_one :: proc "c" (divisor: i128) -> i128 {
	return min(i128) % divisor
}

main :: proc() {}
