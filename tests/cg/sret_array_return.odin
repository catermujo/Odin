package main

Array :: [9]f32

array_return :: proc() -> Array {
	result: Array
	result[0] = 1
	return result
}

main :: proc() {
	_ = array_return()
}
