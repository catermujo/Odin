package main

import "core:math/bits"

byte_swap :: bits.byte_swap

@(export)
swap_u16 :: proc(value: u16) -> u16 {
	return byte_swap(value)
}

@(export)
swap_u64 :: proc(value: u64) -> u64 {
	return byte_swap(value)
}

@(export)
swap_u128 :: proc(value: u128) -> u128 {
	return byte_swap(value)
}

main :: proc() {}
