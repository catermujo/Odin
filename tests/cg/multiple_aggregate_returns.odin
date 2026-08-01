package main

Vec2 :: [2]f32
UVec2 :: [2]u32

aspect_fit_rect :: #force_inline proc(src, dst: Vec2) -> (offset, size: Vec2) {
	src_safe := Vec2{max(src.x, 1), max(src.y, 1)}
	dst_safe := Vec2{max(dst.x, 1), max(dst.y, 1)}
	scale := min(dst_safe.x / src_safe.x, dst_safe.y / src_safe.y)
	return (dst_safe - src_safe * scale) / 2, src_safe * scale
}

dynamic_target_size :: #force_no_inline proc(enabled: bool, size: UVec2) -> UVec2 {
	out := size
	if enabled {
		_, fit_size := aspect_fit_rect({16, 9}, Vec2(out))
		out = {u32(max(i32(fit_size.x + 0.5), 1)), u32(max(i32(fit_size.y + 0.5), 1))}
	}
	return out
}

main :: proc() {
	_, fit_size := aspect_fit_rect({16, 9}, Vec2(UVec2{1800, 1080}))
	assert(fit_size.x > 1799 && fit_size.x < 1801)
	assert(fit_size.y > 1012 && fit_size.y < 1013)
	assert(dynamic_target_size(false, {1800, 1080}) == {1800, 1080})
	assert(dynamic_target_size(true, {1800, 1080}) == {1800, 1013})
}
