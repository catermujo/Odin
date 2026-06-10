// Test: heterogeneous aggregate return (struct with mixed int+float fields)
//
// AAPCS64 classifies aggregates: HFA (1-4 f32 or 1-2 f64) returns in
// vector regs (v0..vN); a non-HFA like `struct{x: int, y: f32}` (mixed
// int and float) goes via sret. The fast-backend should still handle
// the sret path correctly for these.
//
// Two variants:
//   Hetero12: 12 bytes (int + f32 + f32). 12 doesn't fit in a single
//             x0:x1 register pair, so sret.
//   Hetero16: 16 bytes (int + f64). f64 alone wouldn't be HFA, so sret.
package test_hetero_agg_return

import "core:fmt"

Hetero12 :: struct {
	x: int,
	y: f32,
	z: f32,
}

Hetero16 :: struct {
	x: int,
	y: f64,
}

mk_hetero12 :: proc "c" () -> Hetero12 {
	return Hetero12{42, 3.14, 2.71}
}

mk_hetero16 :: proc "c" () -> Hetero16 {
	return Hetero16{7, 6.28}
}

g_h12: Hetero12
g_h16: Hetero16

do_test :: proc() {
	g_h12 = mk_hetero12()
	g_h16 = mk_hetero16()
}

main :: proc() {
	do_test()
	fmt.println("h12 x =", g_h12.x) // expect 42
	fmt.println("h12 y =", g_h12.y) // expect 3.14
	fmt.println("h12 z =", g_h12.z) // expect 2.71
	fmt.println("h16 x =", g_h16.x) // expect 7
	fmt.println("h16 y =", g_h16.y) // expect 6.28

	if g_h12.x != 42 || g_h12.y != 3.14 || g_h12.z != 2.71 {
		fmt.println("FAIL: h12")
	}
	if g_h16.x != 7 || g_h16.y != 6.28 {
		fmt.println("FAIL: h16")
	}
}
