package main

f16_one: f16 = 1

main :: proc() {
	uvs: [4][2]f16 = {{0, 0}, {0, 1}, {1, 1}, {1, 0}}
	assert(f32(f16_one) == 1)
	for uv, i in uvs {
		expect_x: f32 = 0
		expect_y: f32 = 0
		if i >= 2 do expect_x = 1
		if i == 1 || i == 2 do expect_y = 1
		assert(f32(uv[0]) == expect_x)
		assert(f32(uv[1]) == expect_y)
	}
}
