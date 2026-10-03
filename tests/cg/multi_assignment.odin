package main

swap_digits :: proc(input: ^[2]i32) -> i32 {
	a, b := input[0], input[1]
	a, b = b, a
	return a*10 + b
}

swap_entries :: proc(input: ^[2]i32) -> i32 {
	input[0], input[1] = input[1], input[0]
	return input[0]*10 + input[1]
}

Pixel :: [3]u8

Point :: struct {x, y: i32}

swap_pixels :: proc(input: ^[2]Pixel) -> Pixel {
	input[0], input[1] = input[1], input[0]
	return input[0]
}

swap_points :: proc(input: ^[2]Point) -> Point {
	input[0], input[1] = input[1], input[0]
	return input[0]
}

main :: proc() {
	input := [2]i32{1, 2}
	_ = swap_digits(&input)
	_ = swap_entries(&input)
	pixels := [2]Pixel{{1, 2, 3}, {4, 5, 6}}
	_ = swap_pixels(&pixels)
	points := [2]Point{{1, 2}, {3, 4}}
	_ = swap_points(&points)
}
