package native_memory_volatile_reject

import "base:intrinsics"

@(export)
native_memory_volatile_store :: proc "c" (destination: ^u32, value: u32) {
	intrinsics.volatile_store(destination, value)
}

@(export)
native_memory_volatile_narrow :: proc "c" (destination: ^i8, value: i8) -> i8 {
	intrinsics.volatile_store(destination, value)
	return intrinsics.volatile_load(destination)
}

@(export)
native_memory_volatile_wide :: proc "c" (destination: ^u64, value: u64) -> u64 {
	intrinsics.volatile_store(destination, value)
	return intrinsics.volatile_load(destination)
}

@(export)
native_memory_volatile_float :: proc "c" (destination: ^f64, value: f64) -> f64 {
	intrinsics.volatile_store(destination, value)
	return intrinsics.volatile_load(destination)
}

main :: proc() {}
