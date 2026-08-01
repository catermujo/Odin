package main

import "base:runtime"

copy_string_to_bytes :: #force_inline proc "contextless" (dst: []u8, src: string) -> int {
	return copy(dst, src)
}

append_string_to_bytes :: proc(array: ^$T/[dynamic]$E/u8, src: $S/string) -> (int, runtime.Allocator_Error) {
	return append_string_to_bytes_inner(array, src)
}

append_string_to_bytes_inner :: proc (array: ^$T/[dynamic]$E/u8, src: $S/string) -> (int, runtime.Allocator_Error) {
	return append_elem_string(array, src)
}

main :: proc() {
	bytes := make([]u8, 8)
	dynamic_bytes := make([dynamic]u8)

	copied := copy_string_to_bytes(bytes, "copy")
	appended, err := append_string_to_bytes(&dynamic_bytes, "append")

	assert(copied == 4)
	assert(appended == 6)
	assert(err == nil)

	delete(bytes)
	delete(dynamic_bytes)
}
