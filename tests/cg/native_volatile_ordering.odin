package native_volatile_ordering

import "base:intrinsics"

native_volatile_helper :: proc "contextless" () {}

@(export)
native_volatile_discarded :: proc "c" (first, read, last: ^u32) {
	intrinsics.volatile_store(first, 1)
	_ = intrinsics.volatile_load(read)
	intrinsics.volatile_store(last, 2)
}

@(export)
native_volatile_used_after_store :: proc "c" (read, write: ^u32) -> u32 {
	value := intrinsics.volatile_load(read)
	intrinsics.volatile_store(write, 3)
	return value
}

@(export)
native_volatile_used_after_call :: proc "c" (address: ^u32) -> u32 {
	value := intrinsics.volatile_load(address)
	native_volatile_helper()
	return value
}

@(export)
native_volatile_two_loads :: proc "c" (address: ^u32) -> u32 {
	first := intrinsics.volatile_load(address)
	second := intrinsics.volatile_load(address)
	return first + second
}

@(export)
native_volatile_integer_widths :: proc "c" (bool_ptr: ^bool, i8_ptr: ^i8, i16_ptr: ^i16, i32_ptr: ^i32,
	i64_ptr: ^i64, pointer_ptr: ^^u8) -> i64 {
	bool_value := intrinsics.volatile_load(bool_ptr)
	i8_value := intrinsics.volatile_load(i8_ptr)
	i16_value := intrinsics.volatile_load(i16_ptr)
	i32_value := intrinsics.volatile_load(i32_ptr)
	i64_value := intrinsics.volatile_load(i64_ptr)
	pointer_value := intrinsics.volatile_load(pointer_ptr)
	intrinsics.volatile_store(bool_ptr, bool_value)
	intrinsics.volatile_store(i8_ptr, i8_value)
	intrinsics.volatile_store(i16_ptr, i16_value)
	intrinsics.volatile_store(i32_ptr, i32_value)
	intrinsics.volatile_store(i64_ptr, i64_value)
	intrinsics.volatile_store(pointer_ptr, pointer_value)
	return i64_value
}

@(export)
native_volatile_float_widths :: proc "c" (f32_ptr: ^f32, f64_ptr: ^f64) -> f64 {
	f32_value := intrinsics.volatile_load(f32_ptr)
	f64_value := intrinsics.volatile_load(f64_ptr)
	intrinsics.volatile_store(f32_ptr, f32_value)
	intrinsics.volatile_store(f64_ptr, f64_value)
	return f64(f32_value) + f64_value
}

@(export)
native_volatile_branch :: proc "c" (address: ^u32, enabled: bool) -> u32 {
	if enabled {
		return intrinsics.volatile_load(address)
	}
	return 0
}

@(export)
native_volatile_loop :: proc "c" (address: ^u32, count: u32) -> u32 {
	total := u32(0)
	for i := u32(0); i < count; i += 1 {
		total += intrinsics.volatile_load(address)
	}
	return total
}

@(export)
native_volatile_two_consumers :: proc "c" (address: ^u32) -> u32 {
	value := intrinsics.volatile_load(address)
	return value + value
}

main :: proc() {}
