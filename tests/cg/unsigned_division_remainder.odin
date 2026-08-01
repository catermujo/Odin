package main

import "core:strconv"

test_high_bit_unsigned_division_and_remainder :: proc() {
	value := u64(0xffff_ffff_ffff_fff9)
	base := u64(16)
	assert(value / base == u64(0x0fff_ffff_ffff_ffff))
	assert(value % base == u64(9))
}

test_high_bit_unsigned_formatting :: proc() {
	buf: [32]byte
	text := strconv.write_bits(buf[:], u64(0xffff_ffff_ffff_fff9), 16, false, 64, "0123456789abcdef", nil)
	assert(text == "fffffffffffffff9")
}

main :: proc() {
	test_high_bit_unsigned_division_and_remainder()
	test_high_bit_unsigned_formatting()
}
