// Test: #aligned struct as proc return
//
// A struct with `#align(N)` for N > 16 might trip the fast-backend
// if it doesn't honor the alignment for the sret buffer. AAPCS64
// doesn't strictly require >16-byte alignment for aggregates, but
// x86-64 SysV requires 16-byte alignment for SSE types.
package test_aligned_agg

import "core:fmt"

// 32-byte aligned struct
Aligned32 :: struct #align(32) {
	a, b, c, d: int,
}

mk :: proc "c" () -> Aligned32 {
	return Aligned32{1, 2, 3, 4}
}

g: Aligned32

do_test :: proc() {
	g = mk()
}

main :: proc() {
	do_test()
	fmt.println("a =", g.a) // expect 1
	fmt.println("b =", g.b) // expect 2
	fmt.println("c =", g.c) // expect 3
	fmt.println("d =", g.d) // expect 4

	if g.a != 1 || g.b != 2 || g.c != 3 || g.d != 4 {
		fmt.println("FAIL")
	}
}
