package main

Mat :: matrix[2, 2]f32

set_and_get :: proc(m: ^Mat) -> f32 {
	m[0, 1] = 3
	return m[0, 1]
}

main :: proc() {
	m: Mat
	_ = set_and_get(&m)
}
