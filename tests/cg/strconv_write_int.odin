package main

import "core:strconv"

format_int :: proc() {
	buf: [32]byte
	_ = strconv.write_int(buf[:], -42, 10)
}

main :: proc() {
	format_int()
}
