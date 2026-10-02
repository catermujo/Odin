package main

abs3 :: proc(x: [3]f32) -> [3]f32 {
	return abs(x)
}

main :: proc() {
	assert(abs3([3]f32{-1, 0, 2}) == [3]f32{1, 0, 2})
}
