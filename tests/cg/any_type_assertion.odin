package main

Pair :: struct {
	value: i64,
}

@(export)
any_type_assertion :: proc(value: any) -> i64 {
	return value.(Pair).value
}

main :: proc() {}
