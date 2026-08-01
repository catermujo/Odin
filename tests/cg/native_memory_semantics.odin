package native_memory_semantics

import "base:intrinsics"

native_scalar_global: u64
native_scalar_array: [4]u64

Memory_Block :: struct {
	bytes: [16]u8,
}

@(export)
native_memory_global_access :: proc "c" (value: u64) -> u64 {
	native_scalar_global = value
	return native_scalar_global
}

@(export)
native_memory_array_access :: proc "c" (index: int, value: u64) -> u64 {
	native_scalar_array[index] = value
	return native_scalar_array[index]
}

@(export)
native_memory_unaligned_access :: proc "c" (address: ^u8, value: u32) -> u32 {
	destination := intrinsics.ptr_offset(address, 1)
	intrinsics.unaligned_store((^u32)(destination), value)
	return intrinsics.unaligned_load((^u32)(destination))
}

@(export)
native_memory_narrow_i8 :: proc "c" (address: ^i8) -> i8 {
	return address^
}

@(export)
native_memory_narrow_i16 :: proc "c" (address: ^i16) -> i16 {
	return address^
}

@(export)
native_memory_aggregate_copy :: proc "c" (destination, source: ^Memory_Block) {
	intrinsics.mem_copy_non_overlapping(destination, source, size_of(Memory_Block))
}

@(export)
native_memory_dynamic_move :: proc "c" (destination, source: ^u8, size: int) {
	intrinsics.mem_copy(destination, source, size)
}

@(export)
native_memory_local_overlap_move :: proc "c" (size: int) {
	data: [32]u8
	intrinsics.mem_copy(&data[1], &data[0], size)
	intrinsics.mem_copy(&data[0], &data[1], size)
	intrinsics.mem_copy(&data[0], &data[0], size)
}

@(export)
native_memory_local_nonoverlap_move :: proc "c" (size: int) {
	data: [32]u8
	intrinsics.mem_copy(&data[0], &data[16], size)
}

@(export)
native_memory_local_zero_move :: proc "c" () {
	data: [2]u8
	intrinsics.mem_copy(&data[0], &data[1], 0)
}

@(export)
native_memory_dynamic_zero :: proc "c" (destination: ^u8, size: int) {
	intrinsics.mem_zero_volatile(destination, size)
}

native_memory_loop_helper :: proc "contextless" (destination: ^u32, value: u32) {
	intrinsics.volatile_store(destination, value)
}

@(export)
native_memory_loop :: proc "c" (destination: ^u32, count: u32) -> u32 {
	i := u32(0)
	for i < count {
		native_memory_loop_helper(destination, i)
		i += 1
	}
	return i
}

main :: proc() {}
