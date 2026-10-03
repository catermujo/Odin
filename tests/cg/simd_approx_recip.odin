package main

import "core:simd"

recip_f32 :: proc(x: simd.f32x4) -> simd.f32x4 { return simd.approx_recip(x) }
recip_sqrt_f32 :: proc(x: simd.f32x4) -> simd.f32x4 { return simd.approx_recip_sqrt(x) }
recip_f64 :: proc(x: #simd[4]f64) -> #simd[4]f64 { return simd.approx_recip(x) }
recip_sqrt_f64 :: proc(x: #simd[4]f64) -> #simd[4]f64 { return simd.approx_recip_sqrt(x) }
recip_f64_first :: proc(x: #simd[4]f64) -> f64 { return simd.extract(recip_f64(x), 0) }

main :: proc() {
	f32: simd.f32x4 = 1
	f64: #simd[4]f64 = 1
	_ = recip_f32(f32)
	_ = recip_sqrt_f32(f32)
	_ = recip_f64(f64)
	_ = recip_sqrt_f64(f64)
	_ = recip_f64_first(f64)
}
