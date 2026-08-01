package native_memory_aggregate_reject

import "base:intrinsics"

Pair :: struct {
	first: u64,
	second: u64,
}

Nested :: struct {
	pair: Pair,
	tag: u8,
}

Large :: struct {
	words: [3]u64,
}

Padded :: struct {
	tag: u8,
	word: u64,
}

global_padded: Padded

@(export)
native_memory_aggregate_load :: proc "c" (address: ^Pair) -> Pair {
	return address^
}

@(export)
native_memory_aggregate_nested :: proc "c" (address: ^Nested) -> Nested {
	return address^
}

@(export)
native_memory_aggregate_sret :: proc "c" (address: ^Large) -> Large {
	return address^
}

@(export)
native_memory_aggregate_global :: proc "c" () -> Padded {
	return global_padded
}

@(export)
native_memory_aggregate_unaligned_copy :: proc "c" (destination, source: ^u8) {
	intrinsics.mem_copy_non_overlapping(destination, source, size_of(Padded))
}

main :: proc() {}
