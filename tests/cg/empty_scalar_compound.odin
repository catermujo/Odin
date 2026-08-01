package main

Named_Scalar :: distinct i64

main :: proc() {
	id := typeid{}
	scalar := Named_Scalar{}
	assert(id == typeid{})
	assert(scalar == Named_Scalar{})
}
