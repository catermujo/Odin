package main

import "base:intrinsics"

prefetch :: proc(address: rawptr) {
	intrinsics.prefetch_read_instruction(address, 0)
	intrinsics.prefetch_read_data(address, 1)
	intrinsics.prefetch_write_instruction(address, 2)
	intrinsics.prefetch_write_data(address, 3)
}

main :: proc() {
	value := 0
	prefetch(&value)
}
