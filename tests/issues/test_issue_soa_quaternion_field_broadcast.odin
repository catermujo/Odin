package test_issues

import "core:testing"

@(test)
test_soa_fixed_array_field_projection :: proc(t: ^testing.T) {
	vec: #soa[10][2]f32

	for i in 0..<10 {
		vec[i] = [2]f32{f32(i), f32(i + 100)}
	}

	xs: [10]f32 = vec.x
	ys: [10]f32 = vec.y

	for i in 0..<10 {
		testing.expect_value(t, xs[i], f32(i))
		testing.expect_value(t, ys[i], f32(i+100))
	}

	vec.x = 42
	for i in 0..<10 {
		testing.expect_value(t, vec[i].x, 42)
	}
}

@(test)
test_soa_fixed_quaternion_field_projection :: proc(t: ^testing.T) {
	vec: #soa[4]quaternion128

	vec[0] = quaternion(x=1, y=2, z=3, w=4)
	vec[1] = quaternion(x=5, y=6, z=7, w=8)
	vec[2] = quaternion(x=9, y=10, z=11, w=12)
	vec[3] = quaternion(x=13, y=14, z=15, w=16)

	xs: [4]f32 = vec.x
	ws: [4]f32 = vec.w

	testing.expect_value(t, xs, [4]f32{1, 5, 9, 13})
	testing.expect_value(t, ws, [4]f32{4, 8, 12, 16})

	vec.w = 99
	for i in 0..<4 {
		testing.expect_value(t, vec[i].w, 99)
	}
}
