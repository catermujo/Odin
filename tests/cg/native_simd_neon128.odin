package native_simd_neon128

import "base:intrinsics"
import "core:simd"

U8  :: simd.u8x16
I16 :: simd.i16x8
U32 :: simd.u32x4
I64 :: simd.i64x2
F32 :: simd.f32x4
F64 :: simd.f64x2
I64_1 :: #simd[1]i64
U64_1 :: #simd[1]u64
F64_1 :: #simd[1]f64

neon_global_u8: U8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}
neon_global_i128: i128

@(export)
neon_abi_u8 :: proc "c" (value: U8) -> U8 {
	return value
}

@(export)
neon_abi_i16 :: proc "c" (value: I16) -> I16 {
	return value
}

@(export)
neon_abi_u32 :: proc "c" (value: U32) -> U32 {
	return value
}

@(export)
neon_abi_i64 :: proc "c" (value: I64) -> I64 {
	return value
}

@(export)
neon_abi_f32 :: proc "c" (value: F32) -> F32 {
	return value
}

@(export)
neon_abi_f64 :: proc "c" (value: F64) -> F64 {
	return value
}

@(export)
neon_abi_mixed :: proc "c" (a: u64, b: F32, c: U8, d: f64, e: I16) -> U32 {
	_ = a
	_ = b
	_ = d
	_ = c
	_ = e
	return U32{}
}

@(export)
neon_abi_nine :: proc "c" (a, b, c, d, e, f, g, h, i: U8) -> U8 {
	_ = b
	_ = c
	_ = d
	_ = e
	_ = f
	_ = g
	_ = h
	_ = i
	return a
}

@(export)
neon_call_nine :: proc "c" (dst: ^u8, value: U8) {
	result := neon_abi_nine(value, value, value, value, value, value, value, value, value)
	intrinsics.unaligned_store((^U8)(dst), result)
}

Neon_Proc :: proc "c" (value: U8) -> U8

@(export)
neon_direct_call :: proc "c" (value: U8) -> U8 {
	return neon_abi_u8(value)
}

@(export)
neon_indirect_call :: proc "c" (value: U8) -> U8 {
	target: Neon_Proc = neon_abi_u8
	return target(value)
}

@(export)
neon_result_after_call :: proc "c" (value: U8) -> U8 {
	result := neon_abi_u8(value)
	neon_scalar_sink(1)
	return result
}

neon_scalar_sink :: proc "contextless" (value: u32) {
	_ = value
}

@(export)
neon_local_memory :: proc "c" (dst: ^u8, value: U8) {
	local := value
	intrinsics.unaligned_store((^U8)(&local), local)
	loaded := intrinsics.unaligned_load((^U8)(&local))
	intrinsics.unaligned_store((^U8)(dst), loaded)
}

@(export)
neon_global_memory :: proc "c" (dst: ^u8) {
	loaded := intrinsics.unaligned_load((^U8)(&neon_global_u8))
	intrinsics.unaligned_store((^U8)(dst), loaded)
}

@(export)
neon_i128_truncate_pointer :: proc "c" (source: ^i128) -> u64 {
	return u64(source^)
}

@(export)
neon_i128_truncate_global :: proc "c" () -> u32 {
	return u32(neon_global_i128)
}

@(export)
neon_loop_phi :: proc "c" (dst: ^u8, value: U32, count: u32) {
	result := value
	for i := u32(0); i < count; i += 1 {
		result = simd.add(result, U32{1, 1, 1, 1})
	}
	intrinsics.unaligned_store((^U32)(dst), result)
}

@(export)
neon_if_phi :: proc "c" (dst: ^u8, value: U32, choose: bool) {
	result := value
	if choose {
		result = simd.add(value, U32{1, 2, 3, 4})
	} else {
		result = simd.sub(value, U32{4, 3, 2, 1})
	}
	intrinsics.unaligned_store((^U32)(dst), result)
}

@(export)
neon_broadcast_u8 :: proc "c" (dst: ^u8, value: u8) {
	intrinsics.unaligned_store((^U8)(dst), U8(value))
}

@(export)
neon_broadcast_i16 :: proc "c" (dst: ^u8, value: i16) {
	intrinsics.unaligned_store((^I16)(dst), I16(value))
}

@(export)
neon_broadcast_u32 :: proc "c" (dst: ^u8, value: u32) {
	intrinsics.unaligned_store((^U32)(dst), U32(value))
}

@(export)
neon_broadcast_i64 :: proc "c" (dst: ^u8, value: i64) {
	intrinsics.unaligned_store((^I64)(dst), I64(value))
}

@(export)
neon_broadcast_f32 :: proc "c" (dst: ^u8, value: f32) {
	intrinsics.unaligned_store((^F32)(dst), F32(value))
}

@(export)
neon_broadcast_f64 :: proc "c" (dst: ^u8, value: f64) {
	intrinsics.unaligned_store((^F64)(dst), F64(value))
}

@(export)
neon_integer_ops :: proc "c" (dst: ^u8, a: U32, b: U32, count: U32) {
	add := simd.add(a, b)
	sub := simd.sub(add, b)
	mul := simd.mul(sub, U32{2, 3, 4, 5})
	bits := simd.bit_xor(simd.bit_or(mul, a), b)
	shifted := simd.shr(simd.shl(bits, count), count)
	intrinsics.unaligned_store((^U32)(dst), shifted)
}

@(export)
neon_saturating_ops :: proc "c" (dst: ^u8, a: simd.i8x16, b: simd.i8x16) {
	result := simd.saturating_sub(simd.saturating_add(a, b), b)
	intrinsics.unaligned_store((^simd.i8x16)(dst), result)
}

@(export)
neon_integer_compare_select :: proc "c" (dst: ^u8, a, b: U32) {
	eq := simd.lanes_eq(a, b)
	ne := simd.lanes_ne(a, b)
	lt := simd.lanes_lt(a, b)
	le := simd.lanes_le(a, b)
	result := simd.select(eq, simd.bit_or(lt, le), ne)
	intrinsics.unaligned_store((^U32)(dst), result)
}

@(export)
neon_float_ops :: proc "c" (dst: ^u8, a, b: F32) {
	add := simd.add(a, b)
	sub := simd.sub(add, b)
	mul := simd.mul(sub, b)
	div := simd.div(mul, b)
	result := simd.select(simd.lanes_le(a, b), simd.ceil(div), simd.floor(div))
	intrinsics.unaligned_store((^F32)(dst), result)
}

@(export)
neon_float64_ops :: proc "c" (dst: ^u8, a, b: F64) {
	result := simd.max(simd.min(simd.neg(a), b), a)
	result = simd.nearest(result)
	intrinsics.unaligned_store((^F64)(dst), result)
}

@(export)
neon_float64_arithmetic :: proc "c" (dst: ^u8, a, b: F64) {
	result := simd.div(simd.mul(simd.add(a, b), a), b)
	intrinsics.unaligned_store((^F64)(dst), result)
}

@(export)
neon_float_rounding :: proc "c" (dst: ^u8, value: F32) {
	result := simd.ceil(value) + simd.floor(value) + simd.trunc(value) + simd.nearest(value)
	intrinsics.unaligned_store((^F32)(dst), result)
}

@(export)
neon_shuffle_extract_insert :: proc "c" (dst: ^u8, a, b: U32, index: u32, value: u32) {
	shuffled := simd.shuffle(a, b, 3, 4, 1, 7)
	extracted := simd.extract(shuffled, index)
	result := simd.replace(shuffled, index, extracted + value)
	intrinsics.unaligned_store((^U32)(dst), result)
}

@(export)
neon_runtime_swizzle :: proc "c" (dst: ^u8, value: U8, indices: simd.u8x16) {
	result := simd.runtime_swizzle(value, indices)
	intrinsics.unaligned_store((^U8)(dst), result)
}

@(export)
neon_reductions :: proc "c" (dst: ^u8, value: U32) {
	add := simd.reduce_add_ordered(value)
	mul := simd.reduce_mul_ordered(value)
	minimum := simd.reduce_min(value)
	maximum := simd.reduce_max(value)
	bits := simd.reduce_xor(value)
	result := add + mul + minimum + maximum + bits
	intrinsics.unaligned_store((^u32)(dst), result)
}

@(export)
neon_tree_reductions :: proc "c" (dst: ^u8, value: U32) {
	add := simd.reduce_add_bisect(value)
	mul := simd.reduce_mul_bisect(value)
	result := add + mul
	intrinsics.unaligned_store((^u32)(dst), result)
}

@(export)
neon_float_reductions :: proc "c" (dst: ^u8, value: F32) {
	add := simd.reduce_add_ordered(value)
	mul := simd.reduce_mul_ordered(value)
	result := simd.reduce_min(value) + simd.reduce_max(value) + add + mul
	intrinsics.unaligned_store((^f32)(dst), result)
}

@(export)
neon_sibling_values :: proc "c" (dst: ^u8, a, b: U32) {
	left := simd.add(a, b)
	right := simd.sub(a, b)
	result := simd.bit_xor(left, right)
	intrinsics.unaligned_store((^U32)(dst), result)
}

@(export)
neon_signed_compare_i8 :: proc "c" (dst: ^u8, a, b: simd.i8x16) {
	lt := simd.lanes_lt(a, b)
	le := simd.lanes_le(a, b)
	intrinsics.unaligned_store((^U8)(dst), simd.bit_xor(lt, le))
}

@(export)
neon_signed_compare_i16 :: proc "c" (dst: ^u8, a, b: simd.i16x8) {
	lt := simd.lanes_lt(a, b)
	le := simd.lanes_le(a, b)
	intrinsics.unaligned_store((^simd.u16x8)(dst), simd.bit_xor(lt, le))
}

@(export)
neon_signed_compare_i32 :: proc "c" (dst: ^u8, a, b: simd.i32x4) {
	lt := simd.lanes_lt(a, b)
	le := simd.lanes_le(a, b)
	intrinsics.unaligned_store((^U32)(dst), simd.bit_xor(lt, le))
}

@(export)
neon_signed_compare_i64 :: proc "c" (dst: ^u8, a, b: simd.i64x2) {
	lt := simd.lanes_lt(a, b)
	le := simd.lanes_le(a, b)
	intrinsics.unaligned_store((^simd.u64x2)(dst), simd.bit_xor(lt, le))
}

@(export)
neon_float_compare_f32 :: proc "c" (dst: ^u8, a, b: F32) {
	lt := simd.lanes_lt(a, b)
	le := simd.lanes_le(a, b)
	intrinsics.unaligned_store((^U32)(dst), simd.bit_xor(lt, le))
}

@(export)
neon_float_compare_f64 :: proc "c" (dst: ^u8, a, b: F64) {
	lt := simd.lanes_lt(a, b)
	le := simd.lanes_le(a, b)
	intrinsics.unaligned_store((^simd.u64x2)(dst), simd.bit_xor(lt, le))
}

@(export)
neon_select_noncanonical :: proc "c" (dst: ^u8, condition, when_true, when_false: U32) {
	result := simd.select(condition, when_true, when_false)
	intrinsics.unaligned_store((^U32)(dst), result)
}

@(export)
neon_select_mixed_width :: proc "c" (dst: ^u8, c0, c1, c2, c3: u8, when_true, when_false: U32) {
	condition := #simd[4]u8{c0, c1, c2, c3}
	result := simd.select(condition, when_true, when_false)
	intrinsics.unaligned_store((^U32)(dst), result)
}

@(export)
neon_float64_rounding :: proc "c" (dst: ^u8, value: F64) {
	result := simd.ceil(value) + simd.floor(value) + simd.trunc(value) + simd.nearest(value)
	intrinsics.unaligned_store((^F64)(dst), result)
}

@(export)
neon_live_float_reductions :: proc "c" (dst: ^u8, a, b: F32, extra: f32) {
	add := simd.reduce_add_ordered(a)
	mul := simd.reduce_mul_ordered(b)
	local := extra + 1
	intrinsics.unaligned_store((^f32)(dst), add + mul + local)
}

@(export)
neon_scratch_ordering :: proc "c" (dst: ^u8, a, b: U32, indices: simd.u8x16, index: u32) {
	extracted := simd.extract(a, index)
	shuffled := simd.shuffle(a, b, 3, 4, 1, 7)
	swizzled := simd.runtime_swizzle(transmute(U8)shuffled, indices)
	result := simd.replace(transmute(U32)swizzled, 0, extracted)
	intrinsics.unaligned_store((^U32)(dst), result)
}

@(export)
neon_one_lane_i64 :: proc "c" (dst: ^u8, a, b: i64) {
	lhs := I64_1{a}
	rhs := I64_1{b}
	sum := simd.add(lhs, rhs)
	product := simd.mul(sum, rhs)
	mask := simd.lanes_lt(lhs, rhs)
	intrinsics.unaligned_store((^I64_1)(dst), simd.select(mask, product, sum))
}

@(export)
neon_one_lane_u64 :: proc "c" (dst: ^u8, a, b: u64) {
	lhs := U64_1{a}
	rhs := U64_1{b}
	result := simd.sub(simd.add(lhs, rhs), rhs)
	mask := simd.lanes_le(lhs, rhs)
	intrinsics.unaligned_store((^U64_1)(dst), simd.select(mask, result, rhs))
}

@(export)
neon_one_lane_f64 :: proc "c" (dst: ^u8, a, b: f64) {
	lhs := F64_1{a}
	rhs := F64_1{b}
	result := simd.floor(simd.div(simd.mul(simd.add(lhs, rhs), lhs), rhs))
	mask := simd.lanes_lt(lhs, rhs)
	intrinsics.unaligned_store((^F64_1)(dst), simd.select(mask, result, lhs))
}

@(export)
neon_dynamic_phi :: proc "c" (dst: ^u8, value: U32, count, size: int) {
	data := intrinsics.alloca(size, 16)
	result := value
	for i := 0; i < count; i += 1 {
		result = simd.add(result, U32{1, 2, 3, 4})
	}
	data[0] = u8(count)
	intrinsics.unaligned_store((^U32)(dst), result)
}

@(export)
neon_dynamic_ninth_vector :: proc "c" (dst: ^u8, size: int, a, b, c, d, e, f, g, h, i: U8) {
	data := intrinsics.alloca(size, 16)
	data[0] = 1
	result := simd.bit_xor(i, simd.bit_xor(a, h))
	_ = b
	_ = c
	_ = d
	_ = e
	_ = f
	_ = g
	intrinsics.unaligned_store((^U8)(dst), result)
}

neon_dynamic_call_result_callee :: proc "contextless" (value: U32) -> U32 {
	return simd.add(value, U32{1, 2, 3, 4})
}

@(export)
neon_dynamic_call_result :: proc "c" (dst: ^u8, value: U32, size: int) {
	data := intrinsics.alloca(size, 16)
	data[0] = 1
	result := neon_dynamic_call_result_callee(value)
	intrinsics.unaligned_store((^U32)(dst), result)
}

neon_dynamic_hva_callee :: proc "contextless" (size: int, a, b, c, d: U32) -> (U32, U32, U32, U32) {
	data := intrinsics.alloca(size, 16)
	data[0] = 1
	return c, a, d, b
}

@(export)
neon_dynamic_hva_return :: proc "c" (dst: ^u8, size: int, a, b, c, d: U32) {
	r0, r1, r2, r3 := neon_dynamic_hva_callee(size, a, b, c, d)
	result := simd.bit_xor(simd.bit_xor(r0, r1), simd.bit_xor(r2, r3))
	intrinsics.unaligned_store((^U32)(dst), result)
}

neon_permuted_callee :: proc "c" (a, b, c, d: U32) -> U32 {
	return simd.bit_xor(simd.add(a, c), simd.sub(d, b))
}

@(export)
neon_permuted_call :: proc "c" (dst: ^u8, a, b, c, d: U32) {
	result := neon_permuted_callee(d, b, a, c)
	intrinsics.unaligned_store((^U32)(dst), result)
}

@(export)
neon_extract_pressure :: proc "c" (
	dst: ^u8,
	value: U32,
	index: u32,
	a0, a1, a2, a3, a4, a5, a6, a7, a8: u64,
) {
	extracted := simd.extract(value, index)
	sum := a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8
	intrinsics.unaligned_store((^u32)(dst), extracted + u32(sum))
}

main :: proc() {}
