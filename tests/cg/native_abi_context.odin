package native_abi_context

@(export)
native_abi_context_reader :: proc(value: i64) -> i64 {
	return value + i64(context.user_index)
}

@(export)
native_abi_context_call :: proc(value: i64) -> i64 {
	return native_abi_context_reader(value)
}

@(export)
native_abi_contextless :: proc "contextless" (value: i64) -> i64 {
	return value + 1
}

@(export)
native_abi_context_c_control :: proc "c" (value: i64) -> i64 {
	return value + 1
}

main :: proc() {}
