package main

Mode :: enum u8 {
	zero,
	nonzero,
}

mode_to_bool :: proc(mode: Mode) -> bool {
	return bool(mode)
}

main :: proc() {
	assert(!mode_to_bool(.zero))
	assert(mode_to_bool(.nonzero))
}
