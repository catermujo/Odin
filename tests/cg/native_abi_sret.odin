package native_abi_sret

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
native_abi_sret_small :: proc(first, second: u64) -> Small {
	return Small{first = first, second = second}
}

@(export)
native_abi_sret_large :: proc(first, second, third, fourth: u64) -> Large {
	return Large{first = first, second = second, third = third, fourth = fourth}
}

@(export)
native_abi_sret_multi :: proc(first, second: u64) -> (u64, u32) {
	return first, u32(second)
}

@(export)
native_abi_sret_forward :: proc(first, second: u64) -> Large {
	return native_abi_sret_large(first, second, first, second)
}

@(export)
native_abi_sret_mutate :: proc(value: Large) -> Large {
	mutated := value
	mutated.first += 1
	mutated.fourth += mutated.second
	return mutated
}

main :: proc() {}
