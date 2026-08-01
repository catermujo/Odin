package cg

import "base:intrinsics"

atomic_nand :: proc(value: ^u32) -> u32 {
	return intrinsics.atomic_nand(value, 0xff)
}

main :: proc() {
	value := u32(0)
	_ = atomic_nand(&value)
}
