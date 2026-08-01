package main

import "core:c/libc"

call_c_vararg_string :: proc() {
	libc.printf("%s", "")
}

call_c_vararg_cstring :: proc(text: cstring) {
	libc.printf("%s", text)
}

main :: proc() {
	call_c_vararg_string()
	call_c_vararg_cstring("text")
}
