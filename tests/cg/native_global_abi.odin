package native_global_abi

@(export)
native_global_abi_public: u64 = 0x1122334455667788

@(private)
native_global_abi_private: u64 = 0x8877665544332211

@(linkage="internal")
native_global_abi_internal: u64 = 0x0102030405060708

@(export)
native_global_abi_public_array: [2]u64 = {0x10, 0x20}

@(private)
native_global_abi_private_array: [2]u64 = {0x30, 0x40}

@(export)
native_global_abi_read_public :: proc "c" () -> u64 {
	return native_global_abi_public
}

@(export)
native_global_abi_write_public :: proc "c" (value: u64) {
	native_global_abi_public = value
}

@(export)
native_global_abi_read_private :: proc "c" () -> u64 {
	return native_global_abi_private
}

@(export)
native_global_abi_write_private :: proc "c" (value: u64) {
	native_global_abi_private = value
}

@(export)
native_global_abi_read_internal :: proc "c" () -> u64 {
	return native_global_abi_internal
}

@(export)
native_global_abi_write_internal :: proc "c" (value: u64) {
	native_global_abi_internal = value
}

@(export)
native_global_abi_read_public_array :: proc "c" (index: i64) -> u64 #no_bounds_check {
	return native_global_abi_public_array[index]
}

@(export)
native_global_abi_write_public_array :: proc "c" (index: i64, value: u64) #no_bounds_check {
	native_global_abi_public_array[index] = value
}

@(export)
native_global_abi_read_private_array :: proc "c" (index: i64) -> u64 #no_bounds_check {
	return native_global_abi_private_array[index]
}

@(export)
native_global_abi_write_private_array :: proc "c" (index: i64, value: u64) #no_bounds_check {
	native_global_abi_private_array[index] = value
}

main :: proc() {}
