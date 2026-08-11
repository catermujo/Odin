package cg

import "base:intrinsics"
import "core:simd"

fma :: proc(x, y, z: #simd[4]f32) -> #simd[4]f32 {
	return intrinsics.fused_mul_add(x, y, z)
}

approx_recip_sqrt :: proc(x: #simd[2]f64) -> #simd[2]f64 {
	return intrinsics.simd_approx_recip_sqrt(x)
}

main :: proc() {
	x := fma({2, 2, 2, 2}, {3, 3, 3, 3}, {4, 4, 4, 4})
	assert(simd.extract(x, 0) == 10)
	_ = approx_recip_sqrt({1, 1})
}
