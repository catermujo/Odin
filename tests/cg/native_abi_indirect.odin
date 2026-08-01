package native_abi_indirect

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

Nested :: struct {
	head: Small,
	tail: Large,
}

@(export)
native_abi_indirect_small :: proc(value: Small) -> u64 {
	return value.first + value.second
}

@(export)
native_abi_indirect_large :: proc(value: Large) -> u64 {
	return value.first + value.third + value.fourth
}

@(export)
native_abi_indirect_nested :: proc(value: Nested) -> u64 {
	return value.head.first + value.tail.second + value.tail.fourth
}

@(export)
native_abi_indirect_address :: proc(value: ^Large) -> u64 {
	return value.first + value.third + value.fourth
}

main :: proc() {}
