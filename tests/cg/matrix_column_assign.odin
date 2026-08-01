package main

Vec :: [3]f32
Mat :: matrix[3, 3]f32

add_column :: proc(m: ^Mat, v: Vec) -> f32 {
	m[2] += v
	return m[0, 2]
}

main :: proc() {
	m: Mat
	_ = add_column(&m, Vec{1, 2, 3})
}
