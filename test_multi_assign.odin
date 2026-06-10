// Test: simple multi-assign (lhs.count == rhs.count, all leaves)
//
// Fast-backend already supports simple multi-assignment where each
// pair is a one-to-one (lhs, rhs) assign — see the for loop in
// fast_backend_plan_assign_stmt that iterates as->lhs / as->rhs.
//
// What's NOT yet supported is the *tuple-decompose* form
// `(a, b) = some_call()` (lhs.count == 2, rhs.count == 1, the rhs
// is a tuple-returning expression that needs to be decomposed into
// scalars at the call site). That's tracked separately as part of
// the broader "multiple return values" work.
package test_multi_assign

import "core:fmt"

g_a: int
g_b: int
g_c: int
g_d: int

do_test :: proc() {
	x: int = 10
	y: int = 20
	// 2-LHS / 2-RHS, each rhs is a leaf Ident.
	g_a, g_b = x, y
	g_c, g_d = y, x
}

main :: proc() {
	do_test()
	fmt.println("g_a =", g_a) // expect 10
	fmt.println("g_b =", g_b) // expect 20
	fmt.println("g_c =", g_c) // expect 20
	fmt.println("g_d =", g_d) // expect 10

	if g_a != 10 || g_b != 20 || g_c != 20 || g_d != 10 {
		fmt.println("FAIL")
	}
}
