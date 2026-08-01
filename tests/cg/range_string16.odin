package main

sum_string16 :: proc(text: string16) -> i64 {
	total: i64
	for rune, offset in text {
		total += i64(rune) + i64(offset)
	}
	return total
}

sum_string16_reverse :: proc(text: string16) -> i64 {
	total: i64
	#reverse for rune, offset in text {
		total += i64(rune) + i64(offset)
	}
	return total
}

sum_string16_pointer :: proc(text: ^string16) -> i64 {
	total: i64
	for rune, offset in text {
		total += i64(rune) + i64(offset)
	}
	return total
}

sum_string16_pointer_reverse :: proc(text: ^string16) -> i64 {
	total: i64
	#reverse for rune, offset in text {
		total += i64(rune) + i64(offset)
	}
	return total
}

main :: proc() {
	_ = sum_string16
	_ = sum_string16_reverse
	_ = sum_string16_pointer
	_ = sum_string16_pointer_reverse
}
