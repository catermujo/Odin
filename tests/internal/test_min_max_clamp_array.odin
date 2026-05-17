package test_internal

import "core:testing"

@test
test_min_max_clamp_fixed_array :: proc(t: ^testing.T) {
	a := [4]int{1, 9, -4, 7}
	b := [4]int{2, 3, -5, 8}
	c := [4]int{0, 11, -6, 6}

	testing.expect_value(t, min(a, b),    [4]int{1, 3, -5, 7})
	testing.expect_value(t, min(a, b, c), [4]int{0, 3, -6, 6})
	testing.expect_value(t, max(a, b),    [4]int{2, 9, -4, 8})
	testing.expect_value(t, max(a, b, c), [4]int{2, 11, -4, 8})

	x := [4]int{0, 10, 5, -3}
	lo := [4]int{1, 4, 0, -1}
	hi := [4]int{3, 8, 4, 2}
	testing.expect_value(t, clamp(x, lo, hi), [4]int{1, 8, 4, -1})
}

@test
test_min_max_clamp_enumerated_array :: proc(t: ^testing.T) {
	Axis :: enum {X, Y, Z}

	a := [Axis]f32{.X = 1.0, .Y = 4.5, .Z = -2.0}
	b := [Axis]f32{.X = 2.0, .Y = 3.0, .Z = -3.0}

	testing.expect_value(t, min(a, b), [Axis]f32{.X = 1.0, .Y = 3.0, .Z = -3.0})
	testing.expect_value(t, max(a, b), [Axis]f32{.X = 2.0, .Y = 4.5, .Z = -2.0})
	testing.expect_value(
		t,
		clamp(a, [Axis]f32{.X = 1.5, .Y = 2.0, .Z = -2.5}, [Axis]f32{.X = 2.5, .Y = 5.0, .Z = -1.5}),
		[Axis]f32{.X = 1.5, .Y = 4.5, .Z = -2.0},
	)
}

@test
test_abs_fixed_and_enumerated_array :: proc(t: ^testing.T) {
	testing.expect_value(t, abs([4]int{-1, 9, -4, 7}), [4]int{1, 9, 4, 7})
	testing.expect_value(t, abs([3]f32{-1.5, 4.0, -0.25}), [3]f32{1.5, 4.0, 0.25})
	testing.expect_value(t, abs([4]u32{1, 9, 4, 7}), [4]u32{1, 9, 4, 7})

	Axis :: enum {X, Y, Z}
	a := [Axis]i32{.X = -3, .Y = 0, .Z = 9}
	testing.expect_value(t, abs(a), [Axis]i32{.X = 3, .Y = 0, .Z = 9})
}

@test
test_relational_fixed_and_enumerated_array :: proc(t: ^testing.T) {
	a := [4]int{1, 5, 3, 7}
	b := [4]int{2, 5, 1, 8}

	testing.expect_value(t, a <  b, [4]bool{true,  false, false, true})
	testing.expect_value(t, a <= b, [4]bool{true,  true,  false, true})
	testing.expect_value(t, a >  b, [4]bool{false, false, true,  false})
	testing.expect_value(t, a >= b, [4]bool{false, true,  true,  false})

	Axis :: enum {X, Y, Z}
	x := [Axis]f32{.X = 1, .Y = 2, .Z = 3}
	y := [Axis]f32{.X = 0, .Y = 2, .Z = 5}

	testing.expect_value(t, x < y, [Axis]bool{.X = false, .Y = false, .Z = true})
	testing.expect_value(t, x >= y, [Axis]bool{.X = true, .Y = true, .Z = false})
}
