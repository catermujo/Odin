package native_memory_unaligned_reject

import "base:intrinsics"

@(export)
native_memory_unaligned_store :: proc "c" (destination: ^u32, value: u32) {
	intrinsics.unaligned_store(destination, value)
}

@(export)
native_memory_unaligned_load_u16 :: proc "c" (destination: ^u16) -> u16 {
	return intrinsics.unaligned_load(destination)
}

@(export)
native_memory_unaligned_store_u8 :: proc "c" (destination: ^u8, value: u8) {
	intrinsics.unaligned_store(destination, value)
}

@(export)
native_memory_unaligned_load_f32 :: proc "c" (destination: ^f32) -> f32 {
	return intrinsics.unaligned_load(destination)
}

@(export)
native_memory_unaligned_store_f64 :: proc "c" (destination: ^f64, value: f64) {
	intrinsics.unaligned_store(destination, value)
}

main :: proc() {}
