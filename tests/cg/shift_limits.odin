package main

shift_left :: proc(value: u64, count: uint) -> u64 {
	return value << count
}

shift_right_unsigned :: proc(value: u64, count: uint) -> u64 {
	return value >> count
}

shift_right_signed :: proc(value: i64, count: uint) -> i64 {
	return value >> count
}

main :: proc() {
	_ = shift_left(1, 64)
	_ = shift_right_unsigned(1, 64)
	_ = shift_right_signed(-1, 64)
}
