package main

import "core:simd"
import "base:intrinsics"

clamp_f32 :: proc(x, min, max: simd.f32x4) -> simd.f32x4 { return simd.clamp(x, min, max) }
clamp_i32 :: proc(x, min, max: simd.i32x4) -> simd.i32x4 { return simd.clamp(x, min, max) }
pairwise_add_u32 :: proc(x, y: simd.u32x4) -> simd.u32x4 { return simd.pairwise_add(x, y) }
pairwise_sub_i32 :: proc(x, y: simd.i32x4) -> simd.i32x4 { return intrinsics.simd_pairwise_sub(x, y) }

main :: proc() {
	f: simd.f32x4 = 1
	i: simd.i32x4 = 1
	u: simd.u32x4 = 1
	_ = clamp_f32(f, f, f)
	_ = clamp_i32(i, i, i)
	_ = pairwise_add_u32(u, u)
	_ = pairwise_sub_i32(i, i)
}
