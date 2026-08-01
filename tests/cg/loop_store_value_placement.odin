package main

Decimal :: struct {
	digits: [4]u8,
	count:  int,
}

trim :: proc(value: ^Decimal) {
	for value.count > 0 && value.digits[value.count-1] == 0 {
		value.count -= 1
	}
}

main :: proc() {
	value: Decimal
	trim(&value)
}
