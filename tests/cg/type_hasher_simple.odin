package main

import "base:intrinsics"

Pair :: struct {left, right: int}

Composite :: struct {
	values: [2]f64,
	name:   string,
}

Choice :: union {i64, f64}

@(export)
hash_pair :: proc "c" (value: ^Pair, seed: uintptr) -> uintptr {
	hasher := intrinsics.type_hasher_proc(Pair)
	return hasher(value, seed)
}

@(export)
hash_composite :: proc "c" (value: ^Composite, seed: uintptr) -> uintptr {
	hasher := intrinsics.type_hasher_proc(Composite)
	return hasher(value, seed)
}

@(export)
hash_choice :: proc "c" (value: ^Choice, seed: uintptr) -> uintptr {
	hasher := intrinsics.type_hasher_proc(Choice)
	return hasher(value, seed)
}

@(export)
hash_f32 :: proc "c" (value: ^f32, seed: uintptr) -> uintptr {
	hasher := intrinsics.type_hasher_proc(f32)
	return hasher(value, seed)
}

@(export)
hash_complex128 :: proc "c" (value: ^complex128, seed: uintptr) -> uintptr {
	hasher := intrinsics.type_hasher_proc(complex128)
	return hasher(value, seed)
}

@(export)
hash_quaternion256 :: proc "c" (value: ^quaternion256, seed: uintptr) -> uintptr {
	hasher := intrinsics.type_hasher_proc(quaternion256)
	return hasher(value, seed)
}

@(export)
hash_cstring :: proc "c" (value: ^cstring, seed: uintptr) -> uintptr {
	hasher := intrinsics.type_hasher_proc(cstring)
	return hasher(value, seed)
}

main :: proc() {}
