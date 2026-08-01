package native_atomics

import "base:intrinsics"

@(export)
native_atomic_load_u32 :: proc "c" (address: ^u32) -> u32 {
	return intrinsics.atomic_load(address)
}

@(export)
native_atomic_load_u8 :: proc "c" (address: ^u8) -> u8 {
	return intrinsics.atomic_load_explicit(address, .Acquire)
}

@(export)
native_atomic_load_u16 :: proc "c" (address: ^u16) -> u16 {
	return intrinsics.atomic_load_explicit(address, .Consume)
}

@(export)
native_atomic_load_u64 :: proc "c" (address: ^u64) -> u64 {
	return intrinsics.atomic_load_explicit(address, .Relaxed)
}

@(export)
native_atomic_store_u8 :: proc "c" (address: ^u8, value: u8) {
	intrinsics.atomic_store_explicit(address, value, .Relaxed)
}

@(export)
native_atomic_store_u16 :: proc "c" (address: ^u16, value: u16) {
	intrinsics.atomic_store_explicit(address, value, .Seq_Cst)
}

@(export)
native_atomic_store_u64 :: proc "c" (address: ^u64, value: u64) {
	intrinsics.atomic_store_explicit(address, value, .Release)
}

@(export)
native_atomic_exchange_u32 :: proc "c" (address: ^u32, value: u32) -> u32 {
	return intrinsics.atomic_exchange_explicit(address, value, .Acq_Rel)
}

@(export)
native_atomic_exchange_u8 :: proc "c" (address: ^u8, value: u8) -> u8 {
	return intrinsics.atomic_exchange_explicit(address, value, .Acquire)
}

@(export)
native_atomic_add_u64 :: proc "c" (address: ^u64, value: u64) -> u64 {
	return intrinsics.atomic_add_explicit(address, value, .Seq_Cst)
}

@(export)
native_atomic_add_u16 :: proc "c" (address: ^u16, value: u16) -> u16 {
	return intrinsics.atomic_add_explicit(address, value, .Release)
}

@(export)
native_atomic_sub_u32 :: proc "c" (address: ^u32, value: u32) -> u32 {
	return intrinsics.atomic_sub(address, value)
}

@(export)
native_atomic_and_u32 :: proc "c" (address: ^u32, value: u32) -> u32 {
	return intrinsics.atomic_and(address, value)
}

@(export)
native_atomic_nand_u32 :: proc "c" (address: ^u32, value: u32) -> u32 {
	return intrinsics.atomic_nand(address, value)
}

@(export)
native_atomic_or_u32 :: proc "c" (address: ^u32, value: u32) -> u32 {
	return intrinsics.atomic_or(address, value)
}

@(export)
native_atomic_xor_u32 :: proc "c" (address: ^u32, value: u32) -> u32 {
	return intrinsics.atomic_xor(address, value)
}

@(export)
native_atomic_xor_u64 :: proc "c" (address: ^u64, value: u64) -> u64 {
	return intrinsics.atomic_xor_explicit(address, value, .Acquire)
}

@(export)
native_atomic_strong_u64_cas :: proc "c" (address: ^u64, old, new: u64) -> (u64, bool) {
	return intrinsics.atomic_compare_exchange_strong_explicit(address, old, new, .Acq_Rel, .Acquire)
}

@(export)
native_atomic_weak_u16_cas :: proc "c" (address: ^u16, old, new: u16) -> (u16, bool) {
	return intrinsics.atomic_compare_exchange_weak_explicit(address, old, new, .Acquire, .Relaxed)
}

@(export)
native_atomic_pointer_cas :: proc "c" (address: ^^u8, old, new: ^u8) -> (^u8, bool) {
	return intrinsics.atomic_compare_exchange_strong(address, old, new)
}

@(export)
native_atomic_fences :: proc "c" () {
	intrinsics.atomic_thread_fence(.Seq_Cst)
	intrinsics.atomic_signal_fence(.Acquire)
	intrinsics.cpu_relax()
}

main :: proc() {}
