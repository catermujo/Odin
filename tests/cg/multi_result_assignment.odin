package main

pair :: proc() -> (i32, i32) {
	return 1, 2
}

assign_pair :: proc() -> (i32, i32) {
	left, right := i32(0), i32(0)
	left, right = pair()
	_, right = pair()
	return left, right
}

main :: proc() {
	_, _ = assign_pair()
}
