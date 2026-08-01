package main

I64 :: distinct i64

Value :: union {
	I64,
}

main :: proc() {
	value := Value(I64(0x3ff0_0000_0000_0000))
	decoded := transmute(f64)value.(I64)
	assert(decoded == 1.0)
}
