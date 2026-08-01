package main

import "base:intrinsics"

foreign _ {
	opaque :: proc() ---
}

measure :: proc() -> i64 {
	before := intrinsics.read_cycle_counter()
	opaque()
	after := intrinsics.read_cycle_counter()
	return after - before
}

main :: proc() {
	_ = measure()
}
