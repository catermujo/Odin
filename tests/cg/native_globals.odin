package native_globals

@(rodata)
@(export)
public_scalar: u32 = 0x11223344

@(rodata)
@(export)
public_string: string = "native-static-data"

@(rodata)
@(export)
public_aggregate: struct {
	first:  u16,
	second: u32,
} = {first = 0x1122, second = 0x33445566}

@(export)
zero_storage: [32]u8

@(linkage="internal")
internal_scalar: u64 = 0x8877665544332211

@(private)
private_scalar: u64 = 0x0102030405060708

foreign {
	imported_data: u64
	imported_proc :: proc() ---
}

@(export)
local_pointer: ^u32 = &public_scalar

@(export)
external_pointer: ^u64 = &imported_data

main :: proc() {
}
