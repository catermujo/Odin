package main

import NS "core:sys/darwin/Foundation"
import qc "vendor:darwin/QuartzCore"

UVec2 :: [2]u32
Vec2  :: [2]f32

Window :: struct {
	size: UVec2,
}

Metal_Window :: struct {
	using win: Window,
	layer:     rawptr,
}

state: Metal_Window

make_window :: proc() -> Metal_Window {
	return {win = {size = {1800, 1013}}}
}

bridge_window :: proc(win: Metal_Window) -> Metal_Window {
	return win
}

main :: proc() {
	layer := qc.MetalLayer_layer()
	assert(layer != nil)

	want := NS.Size{width = 1800, height = 1013}
	qc.MetalLayer_setDrawableSize(layer, want)
	got := qc.MetalLayer_drawableSize(layer)
	assert(got.width == want.width)
	assert(got.height == want.height)

	state = bridge_window(make_window())
	assert(state.size.x == 1800)
	assert(state.size.y == 1013)
	converted := Vec2(state.size)
	roundtrip := UVec2(converted)
	assert(roundtrip == {1800, 1013})
}
