package main

Matrix :: matrix[4, 4]f32

consume :: proc(value: Matrix) {
	_ = value[0, 0]
}

forward :: proc(value: Matrix) {
	consume(value)
}

main :: proc() {
	value: Matrix
	forward(value)
}
