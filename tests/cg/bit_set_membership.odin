package main

Flag :: enum u8 { A, B, C }
Flags :: bit_set[Flag; i32]

has_flags :: proc(flags: Flags) -> bool {
	return .A in flags && .C not_in flags
}

main :: proc() {
	flags := Flags{.A, .B}
	has_flags(flags)
}
