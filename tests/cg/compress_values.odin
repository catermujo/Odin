package main

Pair :: struct {
	x: f32,
	y: i32,
}

pair :: proc(x: f32, y: i32) -> Pair {
	return compress_values(x, y)
}

array :: proc(a, b, c: i32) -> [3]i32 {
	return compress_values(a, b, c)
}

values :: proc(x: f32, y: i32) -> (f32, i32) {
	return x, y
}

pair_from_values :: proc(x: f32, y: i32) -> Pair {
	return compress_values(values(x, y))
}

main :: proc() {
	first := pair(1, 2)
	second := pair_from_values(3, 4)
	items := array(5, 6, 7)
	_ = first.x + f32(first.y) + second.x + f32(second.y) + f32(items[0])
}
