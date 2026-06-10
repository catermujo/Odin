// Test: heterogeneous aggregate as pass-by-value arg
//
// Companion to test_hetero_agg_return. Tests the caller side: passing
// a struct with mixed int+float fields by value to a cdecl proc.
//
// On AAPCS64 a non-HFA aggregate like `struct{x: int, y: f32}` (mixed
// types) is passed by pointer (caller copies the value to a temp
// and passes the temp's address), not in registers. The fast-backend
// should handle this correctly.
package test_hetero_agg_arg

import "core:fmt"

Hetero :: struct {
	x: int,
	y: f32,
	z: f32,
}

sum_hetero :: proc "c" (h: Hetero) -> int {
	return h.x + int(h.y) + int(h.z)
}

g_in:  Hetero
g_out: int

do_test :: proc() {
	g_in  = Hetero{10, 20, 30}
	g_out = sum_hetero(g_in)
}

main :: proc() {
	do_test()
	fmt.println("g_out =", g_out) // expect 10 + 20 + 30 = 60

	if g_out != 60 {
		fmt.println("FAIL")
	}
}
