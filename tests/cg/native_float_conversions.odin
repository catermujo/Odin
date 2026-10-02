package cg

import "base:intrinsics"

foreign {
	native_m15_foreign_f64 :: proc "c" (i: i64, x: f64) -> f64 ---
	native_m15_foreign_void :: proc "c" (i: i64, x: f32) ---
}

@(export)
native_m15_f32_global: f32 = -0.0
@(export)
native_m15_f64_global: f64 = 0h7ff00000_00000000
@(export)
native_m15_f32_nan_global: f32 = 0h7fc00000
@(export)
native_m15_f64_nan_global: f64 = 0h7ff80000_00000000

@(export)
native_m15_f32_to_f64 :: proc(x: f32) -> f64 {
	return f64(x)
}

@(export)
native_m15_f64_to_f32 :: proc(x: f64) -> f32 {
	return f32(x)
}

@(export)
native_m15_i32_to_f32 :: proc(x: i32) -> f32 {
	return f32(x)
}

@(export)
native_m15_i32_to_f64 :: proc(x: i32) -> f64 {
	return f64(x)
}

@(export)
native_m15_i64_to_f64 :: proc(x: i64) -> f64 {
	return f64(x)
}

@(export)
native_m15_i64_to_f32 :: proc(x: i64) -> f32 {
	return f32(x)
}

@(export)
native_m15_u32_to_f32 :: proc(x: u32) -> f32 {
	return f32(x)
}

@(export)
native_m15_u32_to_f64 :: proc(x: u32) -> f64 {
	return f64(x)
}

@(export)
native_m15_u64_to_f64 :: proc(x: u64) -> f64 {
	return f64(x)
}

@(export)
native_m15_u64_to_f32 :: proc(x: u64) -> f32 {
	return f32(x)
}

@(export)
native_m15_f32_to_i32 :: proc(x: f32) -> i32 {
	return i32(x)
}

@(export)
native_m15_f32_to_i64 :: proc(x: f32) -> i64 {
	return i64(x)
}

@(export)
native_m15_f64_to_i64 :: proc(x: f64) -> i64 {
	return i64(x)
}

@(export)
native_m15_f64_to_i32 :: proc(x: f64) -> i32 {
	return i32(x)
}

@(export)
native_m15_f32_to_u32 :: proc(x: f32) -> u32 {
	return u32(x)
}

@(export)
native_m15_f32_to_u64 :: proc(x: f32) -> u64 {
	return u64(x)
}

@(export)
native_m15_f64_to_u64 :: proc(x: f64) -> u64 {
	return u64(x)
}

@(export)
native_m15_f64_to_u32 :: proc(x: f64) -> u32 {
	return u32(x)
}

@(export)
native_m15_sqrt_f32 :: proc(x: f32) -> f32 {
	return intrinsics.sqrt(x)
}

@(export)
native_m15_sqrt_f64 :: proc(x: f64) -> f64 {
	return intrinsics.sqrt(x)
}

@(export)
native_m15_fma_f32 :: proc(a, b, c: f32) -> f32 {
	return intrinsics.fused_mul_add(a, b, c)
}

@(export)
native_m15_fma_f64 :: proc(a, b, c: f64) -> f64 {
	return intrinsics.fused_mul_add(a, b, c)
}

@(export)
native_m15_min_f32 :: proc(a, b: f32) -> f32 {
	return min(a, b)
}

@(export)
native_m15_max_f32 :: proc(a, b: f32) -> f32 {
	return max(a, b)
}

@(export)
native_m15_min_f64 :: proc(a, b: f64) -> f64 {
	return min(a, b)
}

@(export)
native_m15_max_f64 :: proc(a, b: f64) -> f64 {
	return max(a, b)
}

@(export)
native_m15_mixed :: proc(i: i32, u: u64, x: f32, y: f64) -> f64 {
	local: f64 = f64(i) + f64(u)
	local = local + f64(x) + y
	return local
}

@(export)
native_m15_global_math :: proc(x: f32) -> f64 {
	return f64(x) + f64(native_m15_f32_global) + native_m15_f64_global
}

@(export)
native_m15_global_store :: proc(x: i32) -> f64 {
	native_m15_f32_global = f32(x)
	native_m15_f64_global = f64(x)
	return native_m15_f64_global
}

@(export)
native_m15_call :: proc(i: i32, x: f32) -> f64 {
	return native_m15_foreign_f64(i64(i), f64(x))
}

@(export)
native_m15_call_void :: proc(i: i32, x: f32) {
	native_m15_foreign_void(i64(i), x)
}

@(export)
native_m15_loop :: proc(n: i32, x: f32) -> f64 {
	value := x
	for i: i32 = 0; i < n; i += 1 {
		value = value + f32(i)
	}
	return f64(value)
}

main :: proc() {
}
