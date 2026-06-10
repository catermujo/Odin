// Test: multiple return values
//
// Fast-backend currently rejects procs with more than one result
// (see `error(e->token, "Fast backend currently only supports
// procedures with at most one result")` in
// fast_backend_plan_leaf_proc). This test exercises multi-return
// procs and documents the current state.
package test_multi_return

import "core:fmt"

// Two-int tuple return. AAPCS64: a 16-byte aggregate returned in
// x0:x1 (or via sret, depending on planner choice).
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
