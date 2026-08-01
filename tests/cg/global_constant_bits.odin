package main

Flag :: enum u8 { A = 4, B, C }
Flags :: bit_set[Flag; u64]
Bits128 :: bit_set[0..<128; u128]

Version :: bit_field u32 {
	patch: u32 | 16,
	minor: u32 | 8,
	major: u32 | 8,
}

@(rodata)
flags: Flags = {.A, .C}

@(rodata)
flags128: Bits128 = {0, 127}

@(rodata)
version: Version = {patch = 0xabcd, minor = 0x12, major = 0x34}

@(export)
global_constant_bits :: proc "c" () -> u64 {
	if .A in flags && 127 in flags128 {
		return u64(version.patch)
	}
	return 0
}

main :: proc() {}
