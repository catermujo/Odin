package main

Pair :: struct {
	x, y: f32,
}

Single :: struct {
	value: i32,
}

expand_struct :: proc(value: Pair) -> f32 {
	x, y := expand_values(value)
	return x + y
}

expand_array :: proc(value: [2]f32) -> f32 {
	x, y := expand_values(value)
	return x + y
}

expand_single :: proc(value: Single) -> i32 {
	return expand_values(value)
}

main :: proc() {
	_ = expand_struct({1, 2})
	_ = expand_array({3, 4})
	_ = expand_single({5})
}
