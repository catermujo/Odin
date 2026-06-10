// Test: large (>16 bytes) aggregate as pass-by-value arg
//
// 32-byte struct passed by value to a cdecl proc. AAPCS64 passes
// non-HFA aggregates >16 bytes by pointer.
package test_large_agg_arg

import "core:fmt"

Big :: struct {
	a, b, c, d: int, // 32 bytes total
}

sum_big :: proc "c" (b: Big) -> int {
	return b.a + b.b + b.c + b.d
}

g_b:  Big
g_out: int

do_test :: proc() {
	g_b   = Big{1, 2, 3, 4}
	g_out = sum_big(g_b)
}

main :: proc() {
	do_test()
	fmt.println("g_out =", g_out) // expect 1+2+3+4 = 10

	if g_out != 10 {
		fmt.println("FAIL")
	}
}
