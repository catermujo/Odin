package main

blend :: proc(weight: f32) -> [3]f32 {
	left := [3]f32{1, 2, 3}
	right := [3]f32{4, 5, 6}
	return (1.0-weight)*left + weight*right
}

array_equal :: proc(value: [3]f32) -> bool {
	return value != [3]f32{}
}

main :: proc() {
	value := blend(0.25)
	if array_equal(value) {
		_ = value
	}
}
