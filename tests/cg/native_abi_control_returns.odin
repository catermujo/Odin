package native_abi_control_returns

Small :: struct {
	first: u64,
	second: u64,
}

Large :: struct {
	first: u64,
	second: u64,
	third: u64,
	fourth: u64,
}

@(export)
native_abi_control_small :: proc(flag: u64, first, second: u64) -> Small {
	if flag > 0 {
		return Small{first = first, second = second}
	}
	return Small{first = second, second = first}
}

@(export)
native_abi_control_large :: proc(flag: u64, first, second: u64) -> Large {
	if flag > 0 {
		return Large{first = first, second = second, third = first, fourth = second}
	}
	return Large{first = second, second = first, third = second, fourth = first}
}

@(export)
native_abi_control_string :: proc(flag: u64, first, second: string) -> string {
	if flag > 0 {
		return first
	}
	return second
}

main :: proc() {}
