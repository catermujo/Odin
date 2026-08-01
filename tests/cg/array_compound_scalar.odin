package main

scale :: proc(value: [2]f32, factor: f32) -> [2]f32 {
	result := value
	result *= factor
	return result
}

main :: proc() {
	_ = scale({1, 2}, 0.5)
}
