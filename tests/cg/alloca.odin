package main

import "base:intrinsics"

alloc :: proc(size: int) -> []byte {
	return intrinsics.alloca(size, 16)[:size]
}

alloc_offset :: proc(size: int) -> []byte {
	return intrinsics.alloca(size, 16)[1:size]
}

alloc_in_branch :: proc(size: int, pick_first: bool) -> [^]u8 {
	if pick_first {
		return intrinsics.alloca(size, 32)
	}
	return intrinsics.alloca(size+1, 32)
}

main :: proc() {
	_ = alloc(7)
	_ = alloc_offset(7)
	_ = alloc_in_branch(7, true)
}
