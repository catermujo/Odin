package main

import "base:intrinsics"

reverse :: proc(value: u16) -> u16 {
	return intrinsics.reverse_bits(value)
}

main :: proc() {
	reverse(0x0031)
}
