package main

Pair :: struct {
	value: i64,
}

Other :: struct {
	value: f64,
}

Pair_Union :: union {Pair, Other}

@(export)
union_type_assertion :: proc(value: Pair_Union) -> i64 {
	return value.(Pair).value
}

main :: proc() {}
