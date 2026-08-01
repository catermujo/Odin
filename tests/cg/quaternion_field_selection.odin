package main

check_quaternion64 :: proc(value: quaternion64) {
	assert(value.x == 1)
	assert(value.y == 2)
	assert(value.z == 3)
	assert(value.w == 4)
	assert(value.xyz == [3]f16{1, 2, 3})

	pointer_value := value
	pointer := &pointer_value
	assert(pointer^.x == 1)
	assert(pointer^.y == 2)
	assert(pointer^.z == 3)
	assert(pointer^.w == 4)
	assert(pointer^.xyz == [3]f16{1, 2, 3})
}

check_quaternion128 :: proc(value: quaternion128) {
	assert(value.x == 1)
	assert(value.y == 2)
	assert(value.z == 3)
	assert(value.w == 4)
	assert(value.xyz == [3]f32{1, 2, 3})

	pointer_value := value
	pointer := &pointer_value
	assert(pointer^.x == 1)
	assert(pointer^.y == 2)
	assert(pointer^.z == 3)
	assert(pointer^.w == 4)
	assert(pointer^.xyz == [3]f32{1, 2, 3})
}

check_quaternion256 :: proc(value: quaternion256) {
	assert(value.x == 1)
	assert(value.y == 2)
	assert(value.z == 3)
	assert(value.w == 4)
	assert(value.xyz == [3]f64{1, 2, 3})

	pointer_value := value
	pointer := &pointer_value
	assert(pointer^.x == 1)
	assert(pointer^.y == 2)
	assert(pointer^.z == 3)
	assert(pointer^.w == 4)
	assert(pointer^.xyz == [3]f64{1, 2, 3})
}

main :: proc() {
	check_quaternion64(quaternion(w = 4, x = 1, y = 2, z = 3))
	check_quaternion128(quaternion(w = 4, x = 1, y = 2, z = 3))
	check_quaternion256(quaternion(w = 4, x = 1, y = 2, z = 3))
}
