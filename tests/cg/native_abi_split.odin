package native_abi_split

values: [4]u32

@(export)
native_abi_split_string :: proc(value: string) -> string {
	return value
}

@(export)
native_abi_split_slice :: proc(value: []u32) -> []u32 {
	return value
}

@(export)
native_abi_split_mixed :: proc(value: string, numbers: []u32, integer: i64, real: f64) -> (string, []u32) {
	_ = integer
	_ = real
	return native_abi_split_string(value), native_abi_split_slice(numbers)
}

main :: proc() {}
