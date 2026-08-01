package main

split_values :: proc() -> (u32, u32) {
	return 3, 5
}

split_forward :: proc() -> (u32, u32) {
	return split_values()
}

main :: proc() {
	split_forward()
}
