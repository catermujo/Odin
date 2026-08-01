package main

Pair :: struct {
	value: i64,
}

Other :: struct {
	value: f64,
}

Pair_Union :: union {
	Pair,
	Other,
}

@(export)
union_type_assertion_ok :: proc(value: Pair_Union) -> i64 {
	pair, ok := value.(Pair)
	if ok {
		return pair.value
	}
	return 0
}

@(export)
any_type_assertion_ok :: proc(value: any) -> i64 {
	pair, ok := value.(Pair)
	if ok {
		return pair.value
	}
	return 0
}

main :: proc() {
}
