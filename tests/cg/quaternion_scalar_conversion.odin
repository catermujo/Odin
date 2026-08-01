package main

import "core:math/linalg"

check64 :: proc(value: quaternion64, scalar: f16) {
	assert(value.x == 0)
	assert(value.y == 0)
	assert(value.z == 0)
	assert(value.w == scalar)
}

check128 :: proc(value: quaternion128, scalar: f32) {
	assert(value.x == 0)
	assert(value.y == 0)
	assert(value.z == 0)
	assert(value.w == scalar)
}

check_linalg128 :: proc(value: linalg.Quaternionf32, scalar: f32) {
	assert(value.x == 0)
	assert(value.y == 0)
	assert(value.z == 0)
	assert(value.w == scalar)
}

check256 :: proc(value: quaternion256, scalar: f64) {
	assert(value.x == 0)
	assert(value.y == 0)
	assert(value.z == 0)
	assert(value.w == scalar)
}

main :: proc() {
	value64: f16 = 2
	value128: f32 = 3
	value256: f64 = 4
	check64(quaternion64(value64), value64)
	check128(quaternion128(value128), value128)
	check256(quaternion256(value256), value256)
	check_linalg128(linalg.QUATERNIONF32_IDENTITY, 1)
}
