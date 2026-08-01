package native_abi_calls

values: [4]u32

@(export)
native_abi_calls_string_length :: proc(value: string, numbers: []u32, integer: i64) -> i64 {
	return integer + i64(len(value)) + i64(len(numbers))
}

@(export)
native_abi_calls_add :: proc(lhs, rhs: i64) -> i64 {
	return lhs + rhs
}

@(export)
native_abi_calls_chain :: proc(value: string, numbers: []u32, integer: i64) -> i64 {
	local := integer + 1
	first := native_abi_calls_string_length(value, numbers, local)
	return native_abi_calls_add(first, local)
}

@(export)
native_abi_calls_control :: proc(value: string, numbers: []u32, integer: i64) -> i64 {
	if integer > 0 {
		return native_abi_calls_chain(value, numbers, integer)
	}
	return native_abi_calls_add(integer, i64(len(value)))
}

main :: proc() {}
