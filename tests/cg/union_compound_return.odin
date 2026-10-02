package main

Variant :: struct {
	value: int,
}

Token :: union {
	Variant,
}

make_token :: proc(value: int) -> Token {
	return Variant {
		value = value,
	}
}

empty_token :: proc() -> Token {
	return {}
}

main :: proc() {
	_ = make_token(1)
	_ = empty_token()
}
