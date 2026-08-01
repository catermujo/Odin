package main

import "core:simd"

@(export)
round_ceil :: proc(value: #simd[4]f32) -> #simd[4]f32 { return simd.ceil(value) }

@(export)
round_floor :: proc(value: #simd[4]f32) -> #simd[4]f32 { return simd.floor(value) }

@(export)
round_trunc :: proc(value: #simd[4]f32) -> #simd[4]f32 { return simd.trunc(value) }

@(export)
round_nearest :: proc(value: #simd[4]f32) -> #simd[4]f32 { return simd.nearest(value) }

main :: proc() {
	value: #simd[4]f32
	_ = round_ceil(value)
}
