package main

fill :: proc(colors: [4][4]f32) -> f32 {
	return colors[3][3]
}

Color :: [4]f32

red :: Color{1, 0, 0, 1}

fill_named :: proc(colors: [4]Color) -> f32 {
	return colors[0][0] + colors[1][0] + colors[2][0] + colors[3][0]
}

main :: proc() {
	color := f32(2)
	assert(fill(color) == color)
	assert(fill(colors = color) == color)
	assert(fill(red) == 1)
	assert(fill_named(red) == 4)
	assert(fill_named(.5) == 2)
}
