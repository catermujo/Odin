package main

import "base:intrinsics"

utf16_units :: proc() -> int {
	first := intrinsics.constant_utf16_cstring("Aé😀")
	second := intrinsics.constant_utf16_cstring("Aé😀")
	return int(first[0]) + int(first[1]) + int(first[2]) + int(first[3]) + int(first[4]) +
		int(second[0]) + int(second[1]) + int(second[2]) + int(second[3]) + int(second[4])
}

main :: proc() {
	_ = utf16_units()
}
