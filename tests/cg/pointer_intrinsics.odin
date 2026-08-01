package cg

import "base:intrinsics"

pointer_math :: proc(data: ^[8]u32, offset: int) -> int {
	first := &data[0]
	third := intrinsics.ptr_offset(first, offset)
	return intrinsics.ptr_sub(third, first)
}

main :: proc() {
	data: [8]u32
	assert(pointer_math(&data, 2) == 2)
}
