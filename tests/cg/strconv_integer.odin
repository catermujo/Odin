package main

import "core:strconv"

parse_decimal :: proc(value: string) -> (i64, bool) {
	return strconv.parse_i64_of_base(value, 10)
}

main :: proc() {
	_, _ = parse_decimal("1234")
}
