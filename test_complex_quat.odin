// Test: Complex and Quaternion as cdecl proc return
//
// Complex (complex32, complex64, complex128) and Quaternion
// (quaternion64, quaternion128, quaternion256) are `Type_Basic`
// with `BasicFlag_Complex` / `BasicFlag_Quaternion`. They are
// scalar-shaped (handled by `fast_backend_classify_scalar_type`)
// but cdecl procs with these as return types still fall back to
// LLVM — the call setup / sret / flag-based type paths need cases.
// Constructors are complex(re, im) and quaternion(real, imag,
// jmag, kmag) (named-args only for quaternion).
//
// This test verifies the LLVM-fallback round-trip works and
// documents the current state.
package test_complex_quat

import "base:intrinsics"

Cmplx :: complex64
Quat  :: quaternion128

mk_cmplx :: proc "c" () -> Cmplx {
	return complex(3, 4)
}

mk_quat :: proc "c" () -> Quat {
	return quaternion(real=4, imag=1, jmag=2, kmag=3)
}

g_c:  Cmplx
g_q:  Quat

do_test :: proc() {
	g_c = mk_cmplx()
	g_q = mk_quat()
}

main :: proc() {
	do_test()
	if real(g_c) != 3 || imag(g_c) != 4 {
		intrinsics.trap()
	}
	if g_q.x != 1 || g_q.y != 2 || g_q.z != 3 || g_q.w != 4 {
		intrinsics.trap()
	}
}
