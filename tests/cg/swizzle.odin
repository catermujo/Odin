package main

Vec2 :: [2]f32
Vec3 :: [3]f32
Vec4 :: [4]f32

swizzle_xy :: proc(value: Vec3) -> Vec2 {
	return value.xy
}

swizzle_yx :: proc(value: Vec3) -> Vec2 {
	return value.yx
}

swizzle_z :: proc(value: Vec3) -> f32 {
	return value.z
}

index_z :: proc(value: Vec3) -> f32 {
	return value[2]
}

swizzle_z_local :: proc() -> f32 {
	value := Vec3{1, 2, 3}
	return value.z
}

swizzle_zwx :: proc(value: Vec4) -> Vec3 {
	return value.zwx
}

swizzle_bgra :: proc(value: Vec4) -> Vec4 {
	return value.bgra
}

swizzle_ptr_y :: proc(value: ^Vec3) -> f32 {
	return value.y
}

swizzle_assign :: proc(value: ^Vec4, rgb: Vec3, zw: Vec2) {
	value.rgb = rgb
	value.zw = zw
}

main :: proc() {
	_ = swizzle_xy({1, 2, 3})
	_ = swizzle_yx({1, 2, 3})
	_ = swizzle_z({1, 2, 3})
	_ = index_z({1, 2, 3})
	_ = swizzle_z_local()
	_ = swizzle_zwx({1, 2, 3, 4})
	_ = swizzle_bgra({1, 2, 3, 4})
	value := Vec3{1, 2, 3}
	_ = swizzle_ptr_y(&value)
	value4 := Vec4{1, 2, 3, 4}
	swizzle_assign(&value4, {5, 6, 7}, {8, 9})
}
