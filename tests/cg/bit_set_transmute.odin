package main

Flag :: enum i32 {
	A,
	B,
}

Flags :: bit_set[Flag; i32]

from_bits :: #force_inline proc(value: u32) -> Flags {
	return transmute(Flags)(i32(value))
}

main :: proc() {
	from_bits(3)
}
