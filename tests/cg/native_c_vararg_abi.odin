package main

import "base:intrinsics"

foreign import Native_C "system:c"

Large_Result :: struct {
	values: [8]u64,
}

Promotion_Flag :: enum u8 {
	First,
	Second,
	Third,
}

Promotion_Flags :: bit_set[Promotion_Flag; u16]

foreign Native_C {
	native_c_vararg_foreign :: proc "c" (fixed: u64, #c_vararg args: ..any) -> u64 ---
}

native_c_vararg_zero_body :: #force_no_inline proc "c" (fixed: u64, #c_vararg args: ..any) -> u64 {
	list: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	intrinsics.c_va_end(&list)
	return fixed
}

native_c_vararg_promotions_body :: #force_no_inline proc "c" (fixed: i64, #c_vararg args: ..any) -> i64 {
	list: intrinsics.c_va_list
	copy: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	intrinsics.c_va_copy(&copy, &list)
	small_signed := intrinsics.c_va_arg(&list, i32)
	small_unsigned := intrinsics.c_va_arg(&list, i32)
	half_value := intrinsics.c_va_arg(&list, i32)
	flags_value := intrinsics.c_va_arg(&list, i32)
	float_value := intrinsics.c_va_arg(&list, f64)
	copied_signed := intrinsics.c_va_arg(&copy, i32)
	intrinsics.c_va_end(&copy)
	intrinsics.c_va_end(&list)
	return fixed + i64(small_signed) + i64(small_unsigned) + i64(half_value) + i64(flags_value) +
	       i64(float_value) + i64(float_value*2) + i64(copied_signed)
}

native_c_vararg_one_narrow_body :: #force_no_inline proc "c" (
	a0, a1, a2, a3, a4, a5, a6, a7: u64,
	flag: bool,
	#c_vararg args: ..any,
) -> u64 {
	list: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	tail := intrinsics.c_va_arg(&list, u64)
	intrinsics.c_va_end(&list)
	result := a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + tail
	if flag {
		result += 0x101
	}
	return result
}

native_c_vararg_compact_body :: #force_no_inline proc "c" (
	a0, a1, a2, a3, a4, a5, a6, a7: u64,
	flag: bool,
	small: i8,
	half: u16,
	word: u32,
	tail_flag: bool,
	#c_vararg args: ..any,
) -> i64 {
	list: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	first := intrinsics.c_va_arg(&list, i64)
	second := intrinsics.c_va_arg(&list, u64)
	intrinsics.c_va_end(&list)
	result := i64(a0+a1+a2+a3+a4+a5+a6+a7) + i64(small) + i64(half) + i64(word) + first + i64(second)
	if flag {
		result += 0x101
	}
	if tail_flag {
		result += 0x202
	}
	return result
}

native_c_vararg_exhausted_body :: #force_no_inline proc "c" (
	a0, a1, a2, a3, a4, a5, a6, a7: u64,
	f0, f1, f2, f3, f4, f5, f6, f7, f8: f64,
	#c_vararg args: ..any,
) -> u64 {
	list: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	integer := intrinsics.c_va_arg(&list, u64)
	real := intrinsics.c_va_arg(&list, f64)
	intrinsics.c_va_end(&list)
	real_bits := transmute(u64)real
	return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + u64(f0+f1+f2+f3+f4+f5+f6+f7+f8) + integer +
	       real_bits + (real_bits >> 7)
}

native_c_vararg_large_body :: #force_no_inline proc "c" (seed: u64, #c_vararg args: ..any) -> Large_Result {
	list: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	first := intrinsics.c_va_arg(&list, u64)
	second := intrinsics.c_va_arg(&list, u64)
	intrinsics.c_va_end(&list)
	return Large_Result {
		values = {
			seed,
			first,
			second,
			seed + first,
			seed + second,
			first + second,
			seed ~ first,
			seed ~ second,
		},
	}
}

native_c_vararg_cursor_body :: #force_no_inline proc "c" (
	count: int,
	use_copy: bool,
	#c_vararg args: ..any,
) -> i64 {
	list: intrinsics.c_va_list
	copy: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	intrinsics.c_va_copy(&copy, &list)
	cursor := &list
	if use_copy {
		cursor = &copy
	}
	result: i64
	for index := 0; index < count; index += 1 {
		value := intrinsics.c_va_arg(cursor, i64)
		if index & 1 == 0 {
			result += value
		} else {
			result -= value
		}
	}
	intrinsics.c_va_end(&copy)
	intrinsics.c_va_end(&list)
	return result
}

native_c_vararg_dynamic_body :: #force_no_inline proc "c" (
	size: int,
	seed: u64,
	#c_vararg args: ..any,
) -> u64 {
	data := intrinsics.alloca(size+1, 32)[:size+1]
	data[0] = u8(seed)
	list: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	first := intrinsics.c_va_arg(&list, u64)
	second := intrinsics.c_va_arg(&list, u64)
	intrinsics.c_va_end(&list)
	return first + second + u64(data[0])
}

native_c_vararg_many_body :: #force_no_inline proc "c" (count: int, #c_vararg args: ..any) -> u64 {
	list: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	result: u64
	for index := 0; index < count; index += 1 {
		result += intrinsics.c_va_arg(&list, u64)
	}
	intrinsics.c_va_end(&list)
	return result
}

native_c_vararg_pointer_body :: #force_no_inline proc "c" (seed: uintptr, #c_vararg args: ..any) -> uintptr {
	list: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	pointer := intrinsics.c_va_arg(&list, rawptr)
	intrinsics.c_va_end(&list)
	return seed + uintptr(pointer) + (uintptr(pointer) >> 3)
}

native_c_vararg_reuse_helper :: #force_no_inline proc "c" (value: u64) -> u64 {
	return value ~ 0x5a5aa5a5f0f00f0f
}

native_c_vararg_reuse_body :: #force_no_inline proc "c" (
	seed: u64,
	choose_xor: bool,
	#c_vararg args: ..any,
) -> u64 {
	list: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	intrinsics.c_va_copy(&list, &list)
	value := intrinsics.c_va_arg(&list, u64)
	helper := native_c_vararg_reuse_helper(seed)
	result := value + value + helper
	if choose_xor {
		result ~= value
	} else {
		result += value
	}
	intrinsics.c_va_end(&list)
	return result
}

@(export)
native_c_vararg_zero_unnamed :: proc "c" (seed: u64) -> u64 {
	return native_c_vararg_zero_body(seed)
}

@(export)
native_c_vararg_promotions :: proc "c" (seed: i64) -> i64 {
	return native_c_vararg_promotions_body(
		seed,
		i8(-0x12),
		u8(0xe3),
		u16(0xd678),
		Promotion_Flags{.First, .Third},
		f32(2.25),
	)
}

@(export)
native_c_vararg_one_narrow_stack :: proc "c" (seed: u64) -> u64 {
	return native_c_vararg_one_narrow_body(
		seed,
		0x11,
		0x12,
		0x13,
		0x14,
		0x15,
		0x16,
		0x17,
		true,
		u64(0xfedcba9876543210),
	)
}

@(export)
native_c_vararg_compact_stack :: proc "c" (seed: u64) -> i64 {
	return native_c_vararg_compact_body(
		seed,
		0x11,
		0x12,
		0x13,
		0x14,
		0x15,
		0x16,
		0x17,
		true,
		-0x23,
		0xcdef,
		0x89abcdef,
		true,
		i64(-0x123456789),
		u64(0xfedcba9876543210),
	)
}

@(export)
native_c_vararg_gpr_fpr_exhaustion :: proc "c" (seed: u64) -> u64 {
	return native_c_vararg_exhausted_body(
		seed,
		0x21,
		0x22,
		0x23,
		0x24,
		0x25,
		0x26,
		0x27,
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8,
		9,
		u64(0x1122334455667788),
		f64(9.5),
	)
}

@(export)
native_c_vararg_direct_call :: proc "c" (seed: i64) -> i64 {
	return native_c_vararg_promotions_body(
		seed,
		i16(-0x2345),
		u16(0xd678),
		i8(-0x12),
		Promotion_Flags{.Second, .Third},
		f32(4.5),
	)
}

@(export)
native_c_vararg_foreign_call :: proc "c" (seed: u64) -> u64 {
	return native_c_vararg_foreign(seed, u8(0x81), i16(-0x1234), f32(1.25), u64(0x1122334455667788))
}

@(export)
native_c_vararg_pointer_string_call :: proc "c" (seed: u64, text: cstring, pointer: rawptr) -> u64 {
	return native_c_vararg_foreign(seed, "", text, nil, pointer)
}

@(export)
native_c_vararg_indirect_call :: proc "c" (seed: u64) -> u64 {
	call := native_c_vararg_foreign
	return call(seed, bool(true), u16(0xcdef), f32(3.5), u64(0x8877665544332211))
}

@(export)
native_c_vararg_sret :: proc "c" (seed: u64) -> Large_Result {
	return native_c_vararg_large_body(seed, u64(0x123456789abcdef0), u64(0xfedcba9876543210))
}

@(export)
native_c_vararg_branch_loop_cursor :: proc "c" (use_copy: bool) -> i64 {
	return native_c_vararg_cursor_body(6, use_copy, i64(0x10), i64(0x20), i64(0x30), i64(0x40), i64(0x50), i64(0x60))
}

@(export)
native_c_vararg_dynamic_alloca :: proc "c" (size: int, seed: u64) -> u64 {
	return native_c_vararg_dynamic_body(size, seed, u64(0x123456789abcdef0), u64(0xfedcba9876543210))
}

@(export)
native_c_vararg_pointer_result :: proc "c" (seed: uintptr, pointer: rawptr) -> uintptr {
	return native_c_vararg_pointer_body(seed, pointer)
}

@(export)
native_c_vararg_reuse_after_call_join :: proc "c" (seed: u64, choose_xor: bool) -> u64 {
	return native_c_vararg_reuse_body(seed, choose_xor, u64(0x1122334455667788))
}

@(export)
native_c_vararg_forty_unnamed :: proc "c" () -> u64 {
	return native_c_vararg_many_body(
		40,
		u64(0),
		u64(1),
		u64(2),
		u64(3),
		u64(4),
		u64(5),
		u64(6),
		u64(7),
		u64(8),
		u64(9),
		u64(10),
		u64(11),
		u64(12),
		u64(13),
		u64(14),
		u64(15),
		u64(16),
		u64(17),
		u64(18),
		u64(19),
		u64(20),
		u64(21),
		u64(22),
		u64(23),
		u64(24),
		u64(25),
		u64(26),
		u64(27),
		u64(28),
		u64(29),
		u64(30),
		u64(31),
		u64(32),
		u64(33),
		u64(34),
		u64(35),
		u64(36),
		u64(37),
		u64(38),
		u64(39),
	)
}

main :: proc() {}
