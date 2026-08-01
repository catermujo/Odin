package main

import "base:intrinsics"
import "core:simd"

U8x8  :: #simd[8]u8
U8x64 :: #simd[64]u8
I8x64 :: #simd[64]i8
U64x1 :: #simd[1]u64
U32x4 :: #simd[4]u32
U32x8 :: #simd[8]u32
U32x16 :: #simd[16]u32
I32x4 :: #simd[4]i32
I32x8 :: #simd[8]i32
F32x2 :: #simd[2]f32
F32x4 :: #simd[4]f32
F32x8 :: #simd[8]f32
F64x1 :: #simd[1]f64
F64x2 :: #simd[2]f64
F64x4 :: #simd[4]f64
Ptrx4 :: #simd[4]rawptr
Bits64 :: bit_set[0..<64]

native_wide_writable_v256: U32x8 = {0x01020304, 0x11121314, 0x21222324, 0x31323334, 0x41424344, 0x51525354, 0x61626364, 0x71727374}
native_wide_writable_v512: U8x64 = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
}
native_wide_writable_array: [2]U32x8 = {
	{1, 2, 3, 4, 5, 6, 7, 8},
	{9, 10, 11, 12, 13, 14, 15, 16},
}

@(rodata)
native_wide_readonly_v256: U32x8 = {0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88}

@(rodata)
native_wide_readonly_v512: U8x64 = {
	0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
	0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0,
	0xef, 0xee, 0xed, 0xec, 0xeb, 0xea, 0xe9, 0xe8,
	0xe7, 0xe6, 0xe5, 0xe4, 0xe3, 0xe2, 0xe1, 0xe0,
	0xdf, 0xde, 0xdd, 0xdc, 0xdb, 0xda, 0xd9, 0xd8,
	0xd7, 0xd6, 0xd5, 0xd4, 0xd3, 0xd2, 0xd1, 0xd0,
	0xcf, 0xce, 0xcd, 0xcc, 0xcb, 0xca, 0xc9, 0xc8,
	0xc7, 0xc6, 0xc5, 0xc4, 0xc3, 0xc2, 0xc1, 0xc0,
}

foreign {
	native_wide_foreign_v256: U32x8
	native_wide_foreign_v512: U8x64
	native_wide_foreign_v256_call :: proc "c" (value: U32x8) -> U32x8 ---
	native_wide_foreign_v512_call :: proc "c" (value: U8x64) -> U8x64 ---
}

native_wide_v256_callee :: proc "contextless" (value: U32x8) -> U32x8 {
	return value
}

native_wide_v512_callee :: proc "contextless" (value: U8x64) -> U8x64 {
	return value
}

Native_Wide_V256_Proc :: proc "contextless" (value: U32x8) -> U32x8
Native_Wide_V512_Proc :: proc "contextless" (value: U8x64) -> U8x64
Native_Wide_V64_Stacked_Proc :: proc "contextless" (
	a0, a1, a2, a3, a4, a5, a6, a7, a8, a9: U64x1,
) -> U64x1

native_wide_context_callee :: proc "odin" (
	a0, a1, a2, a3, a4, a5, a6, a7: u64,
	value: U32x16,
) -> U32x16 {
	bias := U32x16(u32(a0+a1+a2+a3+a4+a5+a6+a7+u64(context.user_index)))
	return simd.add(value, bias)
}

native_wide_scalar_sink :: proc "contextless" (value: u64) {
	_ = value
}

native_wide_ordering_sink :: proc "contextless" (ptr: rawptr, value: i32) {
	(^i32)(ptr)^ = value
}

native_wide_deinterleave_f64_callee :: proc "contextless" (value: F64x2) -> (F64x1, F64x1) {
	return simd.deinterleave(value, 2)
}

native_wide_deinterleave_u8x64_callee :: proc "odin" (
	value: U8x64,
) -> (U8x8, U8x8, U8x8, U8x8, U8x8, U8x8, U8x8, U8x8) {
	return simd.deinterleave(value, 8)
}

native_wide_v64_stacked_callee :: proc "contextless" (
	a0, a1, a2, a3, a4, a5, a6, a7, a8, a9: U64x1,
) -> U64x1 {
	spilled8 := simd.add(simd.bit_xor(a8, U64x1{0x1122334455667788}), a0)
	spilled9 := simd.add(simd.bit_xor(a9, U64x1{0x8877665544332211}), a1)
	registers := simd.bit_xor(simd.add(a2, a3), simd.add(a4, a5))
	registers = simd.bit_xor(registers, simd.add(a6, a7))
	return simd.add(simd.add(spilled8, spilled9), registers)
}

@(export)
native_wide_global_load_store :: proc "c" (dst: rawptr, index: u64) {
	array_ptr := &native_wide_writable_array[index&1]
	computed := intrinsics.unaligned_load(array_ptr)
	writable256 := intrinsics.unaligned_load(&native_wide_writable_v256)
	readonly256 := intrinsics.unaligned_load(&native_wide_readonly_v256)
	writable512 := intrinsics.unaligned_load(&native_wide_writable_v512)
	readonly512 := intrinsics.unaligned_load(&native_wide_readonly_v512)
	intrinsics.unaligned_store((^U32x8)(dst), simd.bit_xor(computed, simd.bit_xor(writable256, readonly256)))
	intrinsics.unaligned_store((^U8x64)(intrinsics.ptr_offset((^u8)(dst), 32)), simd.bit_xor(writable512, readonly512))
	native_wide_writable_v256 = computed
	native_wide_writable_v512 = writable512
}

@(export)
native_wide_foreign_load_store :: proc "c" (dst: rawptr) {
	v256 := intrinsics.unaligned_load(&native_wide_foreign_v256)
	v512 := intrinsics.unaligned_load(&native_wide_foreign_v512)
	result256 := native_wide_foreign_v256_call(v256)
	result512 := native_wide_foreign_v512_call(v512)
	intrinsics.unaligned_store((^U32x8)(dst), result256)
	intrinsics.unaligned_store((^U8x64)(intrinsics.ptr_offset((^u8)(dst), 32)), result512)
}

@(export)
native_wide_v256_round_trip :: proc "c" (value: U32x8) -> U32x8 {
	return value
}

@(export)
native_wide_v512_round_trip :: proc "c" (value: U8x64) -> U8x64 {
	return value
}

@(export)
native_wide_direct_calls :: proc "c" (value256: U32x8, value512: U8x64, dst: rawptr) {
	result256 := native_wide_v256_callee(value256)
	result512 := native_wide_v512_callee(value512)
	intrinsics.unaligned_store((^U32x8)(dst), result256)
	intrinsics.unaligned_store((^U8x64)(intrinsics.ptr_offset((^u8)(dst), 32)), result512)
}

@(export)
native_wide_indirect_calls :: proc "c" (value256: U32x8, value512: U8x64, dst: rawptr) {
	call256: Native_Wide_V256_Proc = native_wide_v256_callee
	call512: Native_Wide_V512_Proc = native_wide_v512_callee
	result256 := call256(value256)
	result512 := call512(value512)
	intrinsics.unaligned_store((^U32x8)(dst), result256)
	intrinsics.unaligned_store((^U8x64)(intrinsics.ptr_offset((^u8)(dst), 32)), result512)
}

@(export)
native_wide_stacked_context_call :: proc "odin" (value: U32x16) -> U32x16 {
	return native_wide_context_callee(0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, value)
}

@(export)
native_wide_call_result_survival :: proc "c" (value256: U32x8, value512: U8x64, dst: rawptr) {
	result256 := native_wide_v256_callee(value256)
	result512 := native_wide_v512_callee(value512)
	native_wide_scalar_sink(0x1122334455667788)
	intrinsics.unaligned_store((^U32x8)(dst), result256)
	intrinsics.unaligned_store((^U8x64)(intrinsics.ptr_offset((^u8)(dst), 32)), result512)
}

@(export)
native_wide_v64_stacked_direct_call :: proc "c" (
	a0, a1, a2, a3, a4, a5, a6, a7, a8, a9: U64x1,
) -> U64x1 {
	result := native_wide_v64_stacked_callee(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9)
	return simd.bit_xor(result, a9)
}

@(export)
native_wide_v64_stacked_indirect_call :: proc "c" (
	a0, a1, a2, a3, a4, a5, a6, a7, a8, a9: U64x1,
) -> U64x1 {
	call: Native_Wide_V64_Stacked_Proc = native_wide_v64_stacked_callee
	result := call(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9)
	return simd.bit_xor(result, a8)
}

@(export)
native_wide_control_frame :: proc "c" (dst: rawptr, value: U32x16, count, size: int, choose: bool) {
	data := intrinsics.alloca(size+1, 32)[:size+1]
	result := value
	if choose {
		result = simd.bit_xor(result, U32x16(0x55555555))
	} else {
		result = simd.add(result, U32x16(0x11111111))
	}
	for index := 0; index < count; index += 1 {
		result = simd.add(result, U32x16(u32(index+1)))
	}
	data[0] = u8(count)
	intrinsics.unaligned_store((^U32x16)(dst), result)
}

@(export)
native_wide_extract_mask_families :: proc "c" (dst: rawptr) {
	lsb_source := U8x64 {
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	}
	msb_source := I8x64 {
		-128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -128,
		-128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -128,
		-128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -128,
		-128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -128,
	}
	(^Bits64)(dst)^ = simd.extract_lsbs(lsb_source)
	(^Bits64)(intrinsics.ptr_offset((^u8)(dst), 8))^ = simd.extract_msbs(msb_source)
}

@(export)
native_wide_interleave_f64 :: proc "c" (lhs, rhs: F64x1) -> F64x2 {
	return simd.interleave(lhs, rhs)
}

@(export)
native_wide_deinterleave_f64 :: proc "c" (value: F64x2) -> (F64x1, F64x1) {
	first, second := native_wide_deinterleave_f64_callee(value)
	native_wide_scalar_sink(0x21)
	return first, second
}

@(export)
native_wide_interleave_u8x64 :: proc "c" (
	a, b, c, d, e, f, g, h: U8x8,
) -> U8x64 {
	return simd.interleave(a, b, c, d, e, f, g, h)
}

@(export)
native_wide_deinterleave_u8x64 :: proc "odin" (
	value: U8x64,
) -> (U8x8, U8x8, U8x8, U8x8, U8x8, U8x8, U8x8, U8x8) {
	r0, r1, r2, r3, r4, r5, r6, r7 := native_wide_deinterleave_u8x64_callee(value)
	native_wide_scalar_sink(0x87)
	return r0, r1, r2, r3, r4, r5, r6, r7
}

@(export)
native_wide_reciprocal_chain :: proc "c" (dst: rawptr, value32: F32x4, value64: F64x4) {
	recip32 := simd.approx_recip(value32)
	rsqrt32 := simd.approx_recip_sqrt(value32)
	recip64 := simd.approx_recip(value64)
	rsqrt64 := simd.approx_recip_sqrt(value64)
	intrinsics.unaligned_store((^F32x4)(dst), recip32)
	intrinsics.unaligned_store((^F32x4)(intrinsics.ptr_offset((^u8)(dst), 16)), rsqrt32)
	intrinsics.unaligned_store((^F64x4)(intrinsics.ptr_offset((^u8)(dst), 32)), recip64)
	intrinsics.unaligned_store((^F64x4)(intrinsics.ptr_offset((^u8)(dst), 64)), rsqrt64)
}

@(export)
native_wide_sums_chain :: proc "c" (dst: rawptr, i32s: I32x4, f32s4: F32x4, f32s8: F32x8, i8s64: I8x64) {
	sum_i32 := simd.sums_of_n(i32s, 2)
	sum_f32x4 := simd.sums_of_n(f32s4, 4)
	sum_f32x8 := simd.sums_of_n(f32s8, 4)
	sum_i8x64 := simd.sums_of_n(i8s64, 4)
	intrinsics.unaligned_store((^#simd[2]i32)(dst), sum_i32)
	(^f32)(intrinsics.ptr_offset((^u8)(dst), 8))^ = sum_f32x4
	intrinsics.unaligned_store((^F32x2)(intrinsics.ptr_offset((^u8)(dst), 16)), sum_f32x8)
	intrinsics.unaligned_store((^#simd[16]i8)(intrinsics.ptr_offset((^u8)(dst), 32)), sum_i8x64)
}

@(export)
native_wide_nested_scratch :: proc "c" (
	dst: rawptr,
	source: I32x4,
	nested_a, nested_b: #simd[2]i32,
	deinterleave_source: I32x8,
	reduce_source: I32x4,
) {
	shuffled := simd.shuffle(source, simd.interleave(nested_a, nested_b), 0, 5, 2, 7)
	interleaved := simd.interleave(source, simd.interleave(nested_a, nested_b))
	_, deinterleaved := simd.deinterleave(deinterleave_source, 2)
	interleaved_deinterleave := simd.interleave(
		deinterleaved,
		simd.interleave(nested_a, nested_b),
	)
	interleaved_reduce := simd.interleave(
		source,
		simd.replace(
			simd.interleave(nested_a, nested_b),
			3,
			simd.sums_of_n(reduce_source, 4),
		),
	)
	intrinsics.unaligned_store((^I32x4)(dst), shuffled)
	intrinsics.unaligned_store((^I32x8)(intrinsics.ptr_offset((^u8)(dst), 16)), interleaved)
	intrinsics.unaligned_store((^I32x8)(intrinsics.ptr_offset((^u8)(dst), 48)), interleaved_deinterleave)
	intrinsics.unaligned_store((^I32x8)(intrinsics.ptr_offset((^u8)(dst), 80)), interleaved_reduce)
}

@(export)
native_wide_masked_load_store :: proc "c" (src, dst: rawptr, passthrough, values: I32x4) -> I32x4 {
	integer_mask := U32x4{0, 1, 2, 0x80000001}
	boolean_mask := #simd[4]bool{false, true, false, true}
	first := simd.masked_load(src, passthrough, integer_mask)
	second := simd.masked_load(src, first, boolean_mask)
	simd.masked_store(dst, values, integer_mask)
	simd.masked_store(dst, second, boolean_mask)
	return second
}

@(export)
native_wide_masked_expand_compress :: proc "c" (src, dst: rawptr, passthrough, values: I32x4) -> I32x4 {
	mask := U32x4{1, 2, 0x80000000, 1}
	expanded := simd.masked_expand_load(src, passthrough, mask)
	simd.masked_compress_store(dst, values, mask)
	return expanded
}

@(export)
native_wide_gather_scatter :: proc "c" (ptrs: Ptrx4, passthrough, values: I32x4) -> I32x4 {
	mask := U32x4{0, 1, 2, 0x80000001}
	gathered := simd.gather(ptrs, passthrough, mask)
	simd.scatter(ptrs, values, mask)
	return gathered
}

@(export)
native_wide_masked_ordering :: proc "c" (src, dst: rawptr, values: I32x4) -> I32x4 {
	mask := U32x4{1, 0, 0x80000001, 2}
	(^i32)(dst)^ = 0x11223344
	simd.masked_store(dst, values, mask)
	native_wide_ordering_sink(dst, 0x55667788)
	result := simd.masked_load(src, values, mask)
	(^i32)(intrinsics.ptr_offset((^u8)(dst), 16))^ = 0x7f6e5d4c
	return result
}

main :: proc() {}
