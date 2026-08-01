package main

Flag :: enum {
	A = 4,
	B,
	C,
}

Flags :: bit_set[Flag; u64]

transform :: proc(value: Flag) -> Flags {
	flags := Flags{.A, value}
	flags += {.C}
	flags -= {.A}
	return flags + {.B}
}

main :: proc() {
	_ = transform(.B)
}
