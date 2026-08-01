package main

quaternion_magnitude :: proc(value: quaternion128) -> f32 {
	return abs(value)
}

array_minimum :: proc(a, b: [3]f32) -> [3]f32 {
	return min(a, b)
}

array_maximum :: proc(a, b: [3]f32) -> [3]f32 {
	return max(a, b)
}

main :: proc() {
	assert(quaternion_magnitude(quaternion(w = 3, x = 4, y = 0, z = 0)) == 5)

	a := [3]f32{4, -2, 8}
	b := [3]f32{1, 3, 6}
	assert(array_minimum(a, b) == [3]f32{1, -2, 6})
	assert(array_maximum(a, b) == [3]f32{4, 3, 8})
}
