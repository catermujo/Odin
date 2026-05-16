// Tests issue #4210 https://github.com/odin-lang/Odin/issues/4210
package test_issues

import "core:testing"
import "base:intrinsics"

Vec2 :: distinct [2]f32

@test
test_row_major_matrix :: proc(t: ^testing.T) {
	row_major34: #row_major matrix[3,4]int = {
		11,12,13,14,
		21,22,23,24,
		31,32,33,34,
	}
	row_major34_expected := [?]int{11,12,13,14, 21,22,23,24, 31,32,33,34}

	row_major43: #row_major matrix[4,3]int = {
		11,12,13,
		21,22,23,
		31,32,33,
		41,42,43,
	}
	row_major43_expected := [?]int{11,12,13, 21,22,23, 31,32,33, 41,42,43}

	major34_flattened := intrinsics.matrix_flatten(row_major34)
	major34_from_ptr  := intrinsics.unaligned_load((^[3 * 4]int)(&row_major34))

	for row in 0..<3 {
		for column in 0..<4 {
			idx := row * 4 + column
			testing.expect_value(t, major34_flattened[idx], row_major34_expected[idx])
			testing.expect_value(t, major34_from_ptr [idx], row_major34_expected[idx])
		}
	}

	major43_flattened := intrinsics.matrix_flatten(row_major43)
	major43_from_ptr  := intrinsics.unaligned_load((^[4 * 3]int)(&row_major43))

	for row in 0..<4 {
		for column in 0..<3 {
			idx := row * 3 + column
			testing.expect_value(t, major43_flattened[idx], row_major43_expected[idx])
			testing.expect_value(t, major43_from_ptr [idx], row_major43_expected[idx])
		}
	}
}

@test
test_matrix_literal_from_vectors :: proc(t: ^testing.T) {
	col0 := [2]f32{1, 2}
	col1 := [2]f32{3, 4}
	value: matrix[2,2]f32 = {col0, col1}
	expected: matrix[2,2]f32 = {
		1, 3,
		2, 4,
	}
	testing.expect_value(t, value, expected)

	distinct_col0: Vec2 = {5, 6}
	distinct_col1: Vec2 = {7, 8}
	distinct_value: matrix[2,2]f32 = {distinct_col0, distinct_col1}
	distinct_expected: matrix[2,2]f32 = {
		5, 7,
		6, 8,
	}
	testing.expect_value(t, distinct_value, distinct_expected)

}

@test
test_matrix_component_vector_assignment :: proc(t: ^testing.T) {
	m: matrix[2,2]f32

	col0: Vec2 = {11, 22}
	col1: Vec2 = {33, 44}

	m[0] = col0
	m[1] = col1

	testing.expect_value(t, m, matrix[2,2]f32{
		11, 33,
		22, 44,
	})

	recovered0: Vec2 = m[0]
	recovered1: Vec2 = m[1]
	testing.expect_value(t, recovered0[0], col0[0])
	testing.expect_value(t, recovered0[1], col0[1])
	testing.expect_value(t, recovered1[0], col1[0])
	testing.expect_value(t, recovered1[1], col1[1])
}

@test
test_row_major_matrix_component_vectors :: proc(t: ^testing.T) {
	row0 := [3]int{1, 2, 3}
	row1 := [3]int{4, 5, 6}

	value: #row_major matrix[2,3]int = {row0, row1}
	expected: #row_major matrix[2,3]int = {
		1, 2, 3,
		4, 5, 6,
	}
	testing.expect_value(t, value, expected)

	assigned: #row_major matrix[2,3]int
	assigned[0] = row0
	assigned[1] = row1
	testing.expect_value(t, assigned, expected)
}

@test
test_row_minor_matrix :: proc(t: ^testing.T) {
	row_minor34: matrix[3,4]int = {
		11,12,13,14,
		21,22,23,24,
		31,32,33,34,
	}
	row_minor34_expected := [?]int{11,21,31, 12,22,32, 13,23,33, 14,24,34}

	row_minor43: matrix[4,3]int = {
		11,12,13,
		21,22,23,
		31,32,33,
		41,42,43,
	}
	row_minor43_expected := [?]int{11,21,31,41, 12,22,32,42, 13,23,33,43}

	minor34_flattened := intrinsics.matrix_flatten(row_minor34)
	minor34_from_ptr  := intrinsics.unaligned_load((^[3 * 4]int)(&row_minor34))

	for row in 0..<3 {
		for column in 0..<4 {
			idx := row * 4 + column
			testing.expect_value(t, minor34_flattened[idx], row_minor34_expected[idx])
			testing.expect_value(t, minor34_from_ptr [idx], row_minor34_expected[idx])
		}
	}

	minor43_flattened := intrinsics.matrix_flatten(row_minor43)
	minor43_from_ptr  := intrinsics.unaligned_load((^[4 * 3]int)(&row_minor43))

	for row in 0..<4 {
		for column in 0..<3 {
			idx := row * 3 + column
			testing.expect_value(t, minor43_flattened[idx], row_minor43_expected[idx])
			testing.expect_value(t, minor43_from_ptr [idx], row_minor43_expected[idx])
		}
	}
}
