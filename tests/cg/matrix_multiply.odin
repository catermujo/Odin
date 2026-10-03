package main

Matrix :: matrix[3, 3]f32

multiply :: proc(left, right: Matrix) -> Matrix {
	return left * right
}

main :: proc() {
	left, right: Matrix
	_ = multiply(left, right)
}
