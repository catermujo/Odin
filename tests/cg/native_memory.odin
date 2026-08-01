package native_memory

@(export)
native_memory_scalar: u64 = 0x1122334455667788

@(export)
native_memory_array: [4]u64 = {0x10, 0x20, 0x30, 0x40}

@(export)
native_memory_global_read :: proc "c" () -> u64 {
	return native_memory_scalar
}

@(export)
native_memory_global_write :: proc "c" (value: u64) {
	native_memory_scalar = value
}

@(export)
native_memory_pointer_read :: proc "c" (address: ^u64) -> u64 {
	return address^
}

@(export)
native_memory_pointer_write :: proc "c" (address: ^u64, value: u64) {
	address^ = value
}

@(export)
native_memory_array_read_i64 :: proc "c" (index: i64) -> u64 #no_bounds_check {
	return native_memory_array[index]
}

@(export)
native_memory_array_write_i64 :: proc "c" (index: i64, value: u64) #no_bounds_check {
	native_memory_array[index] = value
}

@(export)
native_memory_array_read_constant :: proc "c" () -> u64 {
	return native_memory_array[2]
}

@(export)
native_memory_array_read_i32 :: proc "c" (index: i32) -> u64 #no_bounds_check {
	return native_memory_array[index]
}

@(export)
native_memory_array_write_i32 :: proc "c" (index: i32, value: u64) #no_bounds_check {
	native_memory_array[index] = value
}

main :: proc() {}
