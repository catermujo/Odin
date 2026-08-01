package main

import "core:c/libc"

print_message :: proc(message: string) {
	_ = libc.printf("%s", message)
}

main :: proc() {
	print_message("runtime string")
}
