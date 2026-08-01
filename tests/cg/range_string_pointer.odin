package main

sum_string_pointer :: proc(text: ^string) -> i64 {
	total: i64
	for rune, offset in text {
		total += i64(rune) + i64(offset)
	}
	return total
}

sum_string_pointer_reverse :: proc(text: ^string) -> i64 {
	total: i64
	#reverse for rune, offset in text {
		total += i64(rune) + i64(offset)
	}
	return total
}

main :: proc() {
	_ = sum_string_pointer
	_ = sum_string_pointer_reverse
}
