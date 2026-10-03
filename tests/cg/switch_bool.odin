package main

switch_bool :: proc(input: byte) -> int {
	switch c := input; true {
	case c == 'a':
		return 1
	case '0' <= c && c <= '9':
		return 2
	case:
		return 3
	}
}

main :: proc() {
	_ = switch_bool('a')
}
