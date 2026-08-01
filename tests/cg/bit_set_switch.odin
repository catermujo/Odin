package main

Flag :: enum u8 { Read, Write }
Flags :: bit_set[Flag; u8]

main :: proc() {
	flags := Flags{.Write}
	result := 0
	switch flags & {.Read, .Write} {
	case {.Read}:  result = 1
	case {.Write}: result = 2
	}
	assert(result == 2)
}
