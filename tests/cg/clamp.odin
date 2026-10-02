package main

Vec2 :: [2]f32

min_f32 :: proc(x, y: f32) -> f32 {
	return min(x, y)
}

max_f32 :: proc(x, y: f32) -> f32 {
	return max(x, y)
}

clamp_i32 :: proc(x, low, high: i32) -> i32 {
	return clamp(x, low, high)
}

clamp_u32 :: proc(x, low, high: u32) -> u32 {
	return clamp(x, low, high)
}

clamp_f32 :: proc(x, low, high: f32) -> f32 {
	return clamp(x, low, high)
}

clamp_vec2 :: proc(x, low, high: Vec2) -> Vec2 {
	return clamp(x, low, high)
}

main :: proc() {
	_ = min_f32(-2, 1)
	_ = max_f32(-2, 1)
	_ = clamp_i32(-2, -1, 1)
	_ = clamp_u32(6, 1, 4)
	_ = clamp_f32(-2, -1, 1)
	_ = clamp_vec2({-2, 6}, {-1, 1}, {1, 4})
}
