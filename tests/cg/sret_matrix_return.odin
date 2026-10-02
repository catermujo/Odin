package main

Matrix :: matrix[3, 3]f32

matrix_return :: proc() -> Matrix {
	result: Matrix
	result[0, 0] = 1
	return result
}

main :: proc() {
	_ = matrix_return()
}
