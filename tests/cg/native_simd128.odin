package native_simd128

import "base:intrinsics"

Vec :: #simd[4]u32

@(export)
native_simd_splat_store :: proc "c" (dst: ^u8, seed: u32) {
	v: Vec = seed
	intrinsics.unaligned_store((^Vec)(dst), v)
}

@(export)
native_simd_global_store :: proc "c" (dst: ^u8) {
	intrinsics.unaligned_store((^Vec)(dst), native_simd_global)
}

@(export)
native_simd_ops_store :: proc "c" (dst: ^u8, seed: u32, counts: u32) {
	a: Vec = {seed, 2, 3, 4}
	b: Vec = {5, 6, 7, 8}
	count_vector: Vec = counts
	add := intrinsics.simd_add(a, b)
	xor := intrinsics.simd_bit_xor(add, a)
	left := intrinsics.simd_shl(xor, count_vector)
	right := intrinsics.simd_shr(left, count_vector)
	mask := intrinsics.simd_lanes_lt(count_vector, Vec{32, 32, 32, 32})
	selected := intrinsics.simd_select(mask, right, Vec{0, 0, 0, 0})
	intrinsics.unaligned_store((^Vec)(dst), selected)
}

@(export)
native_simd_loop_store :: proc "c" (dst: ^u8, count: u32) {
	value: Vec = {0, 1, 2, 3}
	for i := u32(0); i < count; i += 1 {
		value = intrinsics.simd_add(value, Vec{1, 1, 1, 1})
	}
	intrinsics.unaligned_store((^Vec)(dst), value)
}

@(export)
native_simd_reuse_store :: proc "c" (dst: ^u8) {
	v0: Vec = {0, 1, 2, 3}
	v1: Vec = {4, 5, 6, 7}
	v2: Vec = {8, 9, 10, 11}
	v3: Vec = {12, 13, 14, 15}
	v4: Vec = {16, 17, 18, 19}
	v5: Vec = {20, 21, 22, 23}
	v6: Vec = {24, 25, 26, 27}
	v7: Vec = {28, 29, 30, 31}
	v8: Vec = {32, 33, 34, 35}
	v9: Vec = {36, 37, 38, 39}
	result := intrinsics.simd_bit_xor(intrinsics.simd_add(v0, v1), intrinsics.simd_add(v8, v9))
	result = intrinsics.simd_bit_xor(result, intrinsics.simd_add(v2, v3))
	result = intrinsics.simd_bit_xor(result, intrinsics.simd_add(v4, v5))
	result = intrinsics.simd_bit_xor(result, intrinsics.simd_add(v6, v7))
	intrinsics.unaligned_store((^Vec)(dst), result)
}

native_simd_global: Vec = {41, 42, 43, 44}

native_simd_round_trip :: proc "contextless" (value: Vec) -> Vec {
	return value
}

native_simd_hva_helper :: proc "contextless" (a, b, c, d: Vec) -> (Vec, Vec, Vec, Vec) {
	return c, a, d, b
}

Simd_HVA_Proc :: proc "contextless" (a, b, c, d: Vec) -> (Vec, Vec, Vec, Vec)

@(export)
native_simd_abi_calls :: proc "c" (dst: ^u8, seed: u32) {
	a: Vec = {seed, 2, 3, 4}
	b: Vec = {5, 6, 7, 8}
	c: Vec = {9, 10, 11, 12}
	d: Vec = {13, 14, 15, 16}
	first_direct := native_simd_round_trip(a)
	first0, first1, first2, first3 := native_simd_hva_helper(a, b, c, d)
	target: Simd_HVA_Proc = native_simd_hva_helper
	second0, second1, second2, second3 := target(d, c, b, a)
	result0 := intrinsics.simd_bit_xor(first_direct, second0)
	result1 := intrinsics.simd_bit_xor(first0, second1)
	result2 := intrinsics.simd_bit_xor(first2, second3)
	result3 := intrinsics.simd_bit_xor(first3, second2)
	result := intrinsics.simd_bit_xor(result0, result1)
	result = intrinsics.simd_bit_xor(result, result2)
	result = intrinsics.simd_bit_xor(result, result3)
	intrinsics.unaligned_store((^Vec)(dst), result)
}

main :: proc() {}
