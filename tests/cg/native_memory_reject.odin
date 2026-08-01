package native_memory_reject

External_Block :: struct {
	first: u64,
	second: u32,
	padding: u8,
}

foreign {
	imported_data: u64
	imported_block: External_Block
}

@(export)
native_memory_external_read :: proc "c" () -> u64 {
	return imported_data
}

@(export)
native_memory_external_write :: proc "c" (value: u64) {
	imported_data = value
}

@(export)
native_memory_external_address :: proc "c" () -> ^u64 {
	return &imported_data
}

@(export)
native_memory_external_aggregate_copy :: proc "c" (destination: ^External_Block) {
	destination^ = imported_block
}

main :: proc() {}
