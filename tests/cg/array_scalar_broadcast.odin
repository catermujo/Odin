package main

Vec :: [2]f32

Rect :: struct {
	pos, size: Vec,
}

scalar_multiply :: proc(value: f32) -> f32 {
	return value * 2
}

integer_multiply :: proc(value: i32) -> i32 {
	return value * 2 + 1
}

integer_shift :: proc(value: u32) -> u32 {
	return value << 1
}

array_value_field :: proc(pos: Vec) -> f32 {
	rect := Rect{pos = pos}
	return rect.pos[0]
}

array_scalar_field :: proc(outer: f32) -> f32 {
	rect := Rect{size = outer * 2}
	return rect.size[0] + rect.size[1]
}

array_scalar_broadcast :: proc(pos: Vec, outer: f32) -> f32 {
	rect := Rect{pos = pos, size = outer * 2}
	return rect.size[0] + rect.size[1]
}

main :: proc() {
	_ = scalar_multiply(3)
	_ = integer_multiply(3)
	_ = integer_shift(3)
	_ = array_value_field({1, 2})
	_ = array_scalar_field(3)
	_ = array_scalar_broadcast({1, 2}, 3)
}
