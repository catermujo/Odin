// Test: Complex and Quaternion as cdecl proc return
//
// Complex (complex32, complex64, complex128) and Quaternion
// (quaternion64, quaternion128, quaternion256) are `Type_Basic`
// with `BasicFlag_Complex` / `BasicFlag_Quaternion` flags. They
// are scalar-shaped (handled by `fast_backend_classify_scalar_type`)
// but the question is whether the cdecl call path / sret path
// / field-access paths handle them.
//
// The constructor functions are `complex64(re, im)` and
// `quaternion(x, y, z, w)`. Compound literals like `{re=3, im=4}`
// are NOT allowed on complex types — see
// `Illegal compound literal, complex64 cannot be used as a
// compound literal with fields`.
//
// This test verifies the round-trip works. Currently expected
// to fall back to LLVM because the field-access and cdecl
// aggregate paths don't have cases for these flag-based types.
package test_complex_quat

import "core:fmt"

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
	fmt.println("re =", real(g_c)) // expect 3
	fmt.println("im =", imag(g_c)) // expect 4
	fmt.println("x =", g_q.x)       // expect 1
	fmt.println("y =", g_q.y)       // expect 2
	fmt.println("z =", g_q.z)       // expect 3
	fmt.println("w =", g_q.w)       // expect 4

	if real(g_c) != 3 || imag(g_c) != 4 {
		fmt.println("FAIL: cmplx")
	}
	if g_q.x != 1 || g_q.y != 2 || g_q.z != 3 || g_q.w != 4 {
		fmt.println("FAIL: quat")
	}
}
