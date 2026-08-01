package main

Mat :: matrix[3, 3]f32

multiply :: proc(left, right: ^Mat) -> f32 {
	left^ *= right^
	return left^[0, 0]
}

main :: proc() {
	left: Mat
	right: Mat
	_ = multiply(&left, &right)
}
