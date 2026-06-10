// Test: #simd vectors as cdecl proc arg/return
//
// Added `case Type_SimdVector:` to
// `fast_backend_type_is_supported_aggregate`, allowing SIMD
// vector types to be planned. The actual ABI handling (HFA
// classification for arm64, xmm-register pass on x86-64) is a
// follow-on — for now the test verifies the round-trip works
// and documents the current fast-emission state.
package test_simd

Vec4 :: #simd[4]f32   // 16 bytes, HFA on AAPCS64 (1-4 f32)

mk_vec4 :: proc "c" () -> Vec4 {
	return Vec4{1.5, 2.25, 3.125, 4.0625}
}

g_v: Vec4

do_test :: proc() {
	g_v = mk_vec4()
}

main :: proc() {
	do_test()
	v: Vec4 = g_v
	if intrinsics.simd_extract(v, 0) != 1.5 ||
	   intrinsics.simd_extract(v, 1) != 2.25 ||
	   intrinsics.simd_extract(v, 2) != 3.125 ||
	   intrinsics.simd_extract(v, 3) != 4.0625 {
		intrinsics.trap()
	}
}

import "base:intrinsics"
