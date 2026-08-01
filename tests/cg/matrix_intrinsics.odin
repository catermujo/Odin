package main

import "base:intrinsics"

matrix_intrinsics :: proc(matrix_value: matrix[2,3]f32, left: [2]f32, right: [3]f32, array_value: [2][3]i32, integers: [2]i32) -> f32 {
	matrix_transpose := intrinsics.transpose(matrix_value)
	array_transpose := intrinsics.transpose(array_value)
	outer := intrinsics.outer_product(left, right)
	matrix_hadamard := intrinsics.hadamard_product(matrix_value, matrix_value)
	array_hadamard := intrinsics.hadamard_product(left, left)
	integer_hadamard := intrinsics.hadamard_product(integers, integers)
	flat := intrinsics.matrix_flatten(matrix_value)
	flat_array := intrinsics.matrix_flatten(array_value)
	return matrix_transpose[0, 0] + f32(array_transpose[0][0]) + outer[0, 0] + matrix_hadamard[0, 0] + array_hadamard[0] + f32(integer_hadamard[0]) + flat[0] + f32(flat_array[0][0])
}

main :: proc() {
	matrix_value: matrix[2,3]f32
	left: [2]f32
	right: [3]f32
	array_value: [2][3]i32
	integers: [2]i32
	_ = matrix_intrinsics(matrix_value, left, right, array_value, integers)
}
