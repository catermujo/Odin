package main

Mask :: bit_set[0 ..< 4]

any_u8 :: proc(value: u8) -> bool {
	return transmute(bool)value
}

any_mask :: proc(value: Mask) -> bool {
	return transmute(bool)value
}

main :: proc() {
	assert(!any_u8(0))
	assert(any_u8(1))
	assert(any_u8(2))
	assert(any_u8(0x80))
	assert(any_mask(transmute(Mask)u8(2)))
}
