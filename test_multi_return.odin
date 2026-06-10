// Test: multiple return values
//
// Fast-backend now supports procs with more than one result
// (`(int, int)`). The proc writes each result to the sret
// buffer at the cumulative offset of that result (a at +0,
// b at +8). The proc body is fast-emitted; the call site
// (tuple-decompose assignment `a, b = call()`) still falls
// back to LLVM, so we test via a LLVM-emitted caller and check
// the fast-emitted callee.
package test_multi_return

import "core:fmt"

mk_pair :: proc "c" (a, b: int) -> (int, int) {
	return a, b
}

g_x: int
g_y: int

do_test :: proc() {
	g_x, g_y = mk_pair(10, 32)
}

main :: proc() {
	do_test()
	fmt.println("g_x =", g_x) // expect 10
	fmt.println("g_y =", g_y) // expect 32

	if g_x != 10 || g_y != 32 {
		fmt.println("FAIL")
	}
}
