package native_macho_linkability

Link_Proc :: proc "contextless" (value: u64) -> u64
Foreign_Proc :: proc "c" (value: u64) -> u64

Link_Const :: struct {
	first: u32,
	second: u64,
}

native_macho_link_const: Link_Const = {0x11223344, 0x5566778899aabbcc}
native_macho_link_data: u64 = 7
native_macho_link_zero: [16]byte
native_macho_link_data_ptr: ^u64 = &native_macho_link_data

native_macho_link_helper :: proc "contextless" (value: u64) -> u64 {
	return value + 1
}

foreign {
	native_macho_link_foreign :: proc "c" (value: u64) -> u64 ---
}

native_macho_link_proc: Link_Proc = native_macho_link_helper
native_macho_link_foreign_proc: Foreign_Proc = native_macho_link_foreign

@(export)
native_macho_link_entry :: proc "c" (value: u64) -> u64 {
	native_macho_link_data = value
	return native_macho_link_helper(value)
}

main :: proc() {}
