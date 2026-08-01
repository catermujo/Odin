package native_memory_atomic_reject

import "base:intrinsics"

@(export)
native_memory_atomic_store :: proc "c" (destination: ^u64, value: u64) {
	intrinsics.atomic_store_explicit(destination, value, .Release)
}

@(export)
native_memory_atomic_load_u8 :: proc "c" (source: ^u8) -> u8 {
	return intrinsics.atomic_load_explicit(source, .Acquire)
}

@(export)
native_memory_atomic_load_u16 :: proc "c" (source: ^u16) -> u16 {
	return intrinsics.atomic_load_explicit(source, .Relaxed)
}

@(export)
native_memory_atomic_load_u32 :: proc "c" (source: ^u32) -> u32 {
	return intrinsics.atomic_load_explicit(source, .Seq_Cst)
}

main :: proc() {}
