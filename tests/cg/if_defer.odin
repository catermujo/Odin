package main

Info :: struct {
	value: i64,
}

@(export)
if_defer_restore :: proc(restore: bool, info: ^Info) {
	if restore {
		previous := info^
		defer info^ = previous
		info.value = 42
	}
}

main :: proc() {
}
