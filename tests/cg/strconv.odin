package main

import "core:strconv"

parse_bool :: proc(value: string) -> bool {
	_, ok := strconv.parse_bool(value)
	return ok
}

main :: proc() {
	_ = parse_bool("true")
}
