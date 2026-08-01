package main

Version :: bit_field u32 {
	patch: uint | 16,
	minor: uint | 8,
	major: uint | 8,
}

version_from_u64 :: proc(value: u64) -> Version {
	return Version(value)
}

main :: proc() {
	_ = version_from_u64(0x030201)
}
