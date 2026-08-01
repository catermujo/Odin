package main

Pair :: struct {
	value: i64,
}

@(export)
address_compound_literal :: proc() -> i64 {
	value := &Pair{value = 42}
	return value.value
}

main :: proc() {
}
