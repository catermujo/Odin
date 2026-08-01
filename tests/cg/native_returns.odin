package native_returns

@(export)
native_void :: proc() {
}

@(export)
native_bool :: proc() -> bool {
	return true
}

@(export)
native_i8 :: proc() -> i8 {
	return -7
}

@(export)
native_u8 :: proc() -> u8 {
	return 0xa5
}

@(export)
native_i16 :: proc() -> i16 {
	return -0x1234
}

@(export)
native_u16 :: proc() -> u16 {
	return 0xbeef
}

@(export)
native_i32 :: proc() -> i32 {
	return -0x1234567
}

@(export)
native_u32 :: proc() -> u32 {
	return 0x89abcdef
}

@(export)
native_i64 :: proc() -> i64 {
	return -0x123456789
}

@(export)
native_u64 :: proc() -> u64 {
	return 0x1122334455667788
}

main :: proc() {
}
