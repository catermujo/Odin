package main

import it "base:intrinsics"

sat_add :: it.saturating_add
sat_sub :: it.saturating_sub

signed_sub :: proc(lhs, rhs: i8) -> i8 {
	return sat_sub(lhs, rhs)
}

unsigned_add :: proc(lhs, rhs: u8) -> u8 {
	return sat_add(lhs, rhs)
}

main :: proc() {
	_ = signed_sub(-100, 100)
	_ = unsigned_add(200, 100)
}
