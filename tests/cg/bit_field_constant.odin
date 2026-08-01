package cg

Version :: bit_field u32 {
	patch: u32 | 16,
	minor: u32 | 8,
	major: u32 | 8,
}

constant :: proc() -> Version {
	return Version(0xffffffff)
}

main :: proc() {
	_ = constant()
}
