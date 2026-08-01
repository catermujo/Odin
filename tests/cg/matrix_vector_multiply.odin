package main

Mat :: matrix[3, 3]f32
Mat4 :: matrix[4, 4]f32

multiply :: proc(m: Mat, v: [3]f32) -> [3]f32 {
	return m * v
}

multiply_scaled :: proc(m: Mat, v: [3]f32, scale, dt: f32) -> [3]f32 {
	return m * v * scale * dt
}

multiply_cast :: proc(m: Mat4, v: [3]f32) -> [3]f32 {
	return Mat(m) * v
}

expand_cast :: proc(m: Mat) -> Mat4 {
	return Mat4(m)
}

step_swizzled :: proc(pos: ^[4]f32, m: Mat4, v: [3]f32, scale, dt: f32) {
	pos.xyz += (Mat(m) * v * scale * dt).xyz
}

main :: proc() {
	m: Mat = 2
	m4: Mat4 = 2
	v := [3]f32{1, 2, 3}
	_ = multiply(m, v)
	_ = multiply_scaled(m, v, 2, 0.5)
	assert(multiply_cast(m4, v) == [3]f32{2, 4, 6})
	expanded := expand_cast(m)
	assert(expanded[0, 0] == 2 && expanded[3, 3] == 1 && expanded[3, 0] == 0)
	pos := [4]f32{}
	step_swizzled(&pos, m4, v, 2, 0.5)
	assert(pos == [4]f32{2, 4, 6, 0})
}
