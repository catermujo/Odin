package main

check :: proc(big_endian: i64be, native: i64) {
	assert(i64(big_endian) == native)
	assert(i64be(native) == big_endian)
}

main :: proc() {
	check(0x0102030405060708, 0x0102030405060708)
}
