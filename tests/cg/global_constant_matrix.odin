package main

column_major: matrix[2, 2]i32 = {1, 2, 3, 4}
row_major: #row_major matrix[2, 2]i32 = {1, 2, 3, 4}
diagonal: matrix[2, 2]i32 = 7
vector_columns: matrix[2, 2]i32 = {[2]i32 {5, 6}, [2]i32 {7, 8}}
vector_rows: #row_major matrix[2, 2]i32 = {[2]i32 {9, 10}, [2]i32 {11, 12}}

@(export)
global_constant_matrix :: proc "c" () -> i32 {
	return column_major[0, 1]*10 + column_major[1, 0] + row_major[0, 1]*100 + row_major[1, 0] + diagonal[0, 0] + diagonal[1, 1] + vector_columns[0, 1]*10 + vector_columns[1, 0] + vector_rows[0, 1]*10 + vector_rows[1, 0]
}

main :: proc() {}
