package main

Vector :: [3]f32

negate :: proc(value: Vector) -> Vector {
	return -value
}

main :: proc() {
	value: Vector
	_ = negate(value)
}
