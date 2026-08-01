package main

main :: proc() {
	a := quaternion(w = 1, x = 2, y = 3, z = 4)
	identity := quaternion(w = 1, x = 0, y = 0, z = 0)

	negated := -a
	assert(negated.w == -1 && negated.x == -2 && negated.y == -3 && negated.z == -4)

	sum := a + identity
	difference := sum - identity
	assert(difference.w == 1 && difference.x == 2 && difference.y == 3 && difference.z == 4)

	product := a * identity
	quotient := a / identity
	assert(product.w == 1 && product.x == 2 && product.y == 3 && product.z == 4)
	assert(quotient.w == 1 && quotient.x == 2 && quotient.y == 3 && quotient.z == 4)

	scaled := a / f64(2)
	assert(scaled.w == .5 && scaled.x == 1 && scaled.y == 1.5 && scaled.z == 2)
}
