package main

main :: proc() {
	v := [4]f32{1, 2, 3, 4}
	v.xz = {9, 7}
	assert(v == {9, 2, 7, 4})
	v.xy = v.yx
	assert(v == {2, 9, 7, 4})
}
