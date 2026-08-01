package main

import "core:c/libc"
import "core:sys/posix"

call_c_varargs :: proc(data: ^u8) {
	_ = libc.printf("%f", f32(1))
	_ = posix.fcntl(0, .GETFL, data)
	_ = posix.open("", {}, {})
}

main :: proc() {
	data: u8
	call_c_varargs(&data)
}
