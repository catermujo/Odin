package main

compare :: proc(value: u8) -> bool {
	return value >= 'a'
}

conjunction :: proc(value: u8) -> bool {
	return value >= 'a' && value <= 'z'
}

bool_to_u8 :: proc(value: u8) -> u8 {
	return u8(conjunction(value))
}

ascii_lower :: proc(value: u8) -> u8 {
	return value - 32 * u8(value >= 'a' && value <= 'z')
}

main :: proc() {
	_ = compare('A')
	_ = conjunction('A')
	_ = bool_to_u8('A')
	_ = ascii_lower('A')
}
