package main

Info :: struct {
	value: i64,
}

@(export)
switch_defer_restore :: proc(verb: rune, info: ^Info) {
	switch verb {
	case 'x':
		previous := info^
		defer info^ = previous
		info.value = 42
	}
}

main :: proc() {
}
