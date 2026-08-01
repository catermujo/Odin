package native_float

foreign {
	native_m14_foreign_void :: proc "c" (i: i32, f: f32, j: i64, d: f64) ---
	native_m14_foreign_f32 :: proc "c" (i: i32, f: f32, j: i64, d: f64) -> f32 ---
	native_m14_foreign_f64 :: proc "c" (i: i32, f: f32, j: i64, d: f64) -> f64 ---
}

@(export)
native_m14_f32_global: f32 = 1.25

@(export)
native_m14_f64_global: f64 = -2.5

@(export)
native_m14_f32_return :: proc "c" (a, b: f32) -> f32 {
	return a + b* f32(2.0)
}

@(export)
native_m14_f64_return :: proc "c" (a, b: f64) -> f64 {
	return a - b* f64(2.0)
}

@(export)
native_m14_f32_neg :: proc "c" (a: f32) -> f32 {
	return -a
}

@(export)
native_m14_f64_neg :: proc "c" (a: f64) -> f64 {
	return -a
}

@(export)
native_m14_f32_sub :: proc "c" (a, b: f32) -> f32 {
	return a - b
}

@(export)
native_m14_f32_div :: proc "c" (a, b: f32) -> f32 {
	return a / b
}

@(export)
native_m14_f64_div :: proc "c" (a, b: f64) -> f64 {
	return a / b
}

@(export)
native_m14_f32_global_memory :: proc "c" (value: f32) -> f32 {
	native_m14_f32_global = value
	return native_m14_f32_global + f32(0.0)
}

@(export)
native_m14_f32_global_read :: proc "c" () -> f32 {
	return native_m14_f32_global
}

@(export)
native_m14_f64_global_memory :: proc "c" () -> f64 {
	native_m14_f64_global = native_m14_f64_global + f64(1.0)
	return native_m14_f64_global
}

@(export)
native_m14_mixed_return :: proc "c" (i: i32, f: f32, j: i64, d: f64) -> f64 {
	return d
}

@(export)
native_m14_foreign_call :: proc "c" (i: i32, f: f32, j: i64, d: f64) -> f64 {
	return native_m14_foreign_f64(i, f, j, d)
}

@(export)
native_m14_foreign_void_call :: proc "c" (i: i32, f: f32, j: i64, d: f64) {
	native_m14_foreign_void(i, f, j, d)
}

@(export)
native_m14_float_compare :: proc "c" (a, b: f64) -> f64 {
	if a == b {
		return f64(1.0)
	} else if a != b {
		return f64(2.0)
	} else if a < b {
		return f64(3.0)
	} else if a <= b {
		return f64(4.0)
	}
	return f64(5.0)
}

@(export)
native_m14_float_phi :: proc "c" (cond: i32, a, b: f64) -> f64 {
	value := a
	if cond != 0 {
		value = b
	}
	return value
}

@(export)
native_m14_float_compare_f32 :: proc "c" (a, b: f32) -> f32 {
	if a < b {
		return f32(1.0)
	}
	if a <= b {
		return f32(2.0)
	}
	if a != b {
		return f32(3.0)
	}
	return f32(4.0)
}

@(export)
native_m14_float_loop :: proc "c" (value, step: f32, count: i32) -> f32 {
	total := value
	i := count
	for i != 0 {
		total = total + step
		i -= 1
	}
	return total
}

@(export)
native_m14_float_local :: proc "c" (value: f64) -> f64 {
	local := value
	local = local * f64(2.0)
	return local
}

@(export)
native_m14_call_result :: proc "c" (i: i32, f: f32, j: i64, d: f64) -> f32 {
	return native_m14_foreign_f32(i, f, j, d)
}

main :: proc() {}
