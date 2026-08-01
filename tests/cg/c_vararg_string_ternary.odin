package main

import "core:c/libc"

print_choice :: proc(flag: bool) {
	_ = libc.printf("%s", flag ? "yes" : "no")
}

main :: proc() {
	print_choice(true)
	print_choice(false)
}
