package main

Kind :: enum int {Zero, One}

plus_kind :: proc(value: Kind) -> Kind {
	return +value
}

plus_float :: proc(value: f64) -> f64 {
	return +value
}

main :: proc() {
	_ = plus_kind(.One)
	_ = plus_float(1.0)
}
