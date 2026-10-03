package main

import "core:strconv"

format_i128 :: proc() {
	buf: [140]byte
	value: i128 = -42
	_ = strconv.write_bits_128(buf[:], u128(value), 10, true, 128, "0123456789", nil)
}

main :: proc() {
	format_i128()
}
