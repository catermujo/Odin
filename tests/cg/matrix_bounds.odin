package main

Mat :: matrix[2, 3]f32

matrix_read :: proc(m: ^Mat, row, column: int) -> f32 {
	return m[row, column]
}

matrix_write :: proc(m: ^Mat, row, column: int) -> f32 {
	m[row, column] = 1
	return m[row, column]
}

matrix_unchecked :: proc(m: ^Mat, row, column: int) -> f32 #no_bounds_check {
	return m[row, column]
}

matrix_column_read :: proc(m: ^Mat, column: int) -> f32 {
	return m[column][0]
}

main :: proc() {
	m: Mat
	_ = matrix_read(&m, 0, 0)
	_ = matrix_write(&m, 0, 0)
	_ = matrix_unchecked(&m, 0, 0)
	_ = matrix_column_read(&m, 0)
}
