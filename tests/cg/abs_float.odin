package main

abs_f32 :: proc(value: f32) -> f32 {
	return abs(value)
}

abs_f64 :: proc(value: f64) -> f64 {
	return abs(value)
}

main :: proc() {
	_ = abs_f32(-1)
	_ = abs_f64(-1)
}
