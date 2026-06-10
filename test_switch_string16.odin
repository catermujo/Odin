// Test: switch on string16
//
// string16 is the rune-counted variant of string (UTF-16). The
// fast-backend's string-switch support (added in
// `fast-backend: support string-typed switch tag`) handles both
// `string` and `string16` via `is_type_string` and
// `is_type_string16` checks. The emit-side string compare path in
// `fast_backend_emit_compare_aggregate_at_offset` uses
// `elem_size = is_type_string16 ? 2 : 1` to size the per-byte
// compare correctly. This test verifies the string16 path works.
package test_switch_string16

import "core:fmt"

g_r: int

classify16 :: proc(s: string16) -> int {
	switch s {
	case "apple":
		return 1
	case "banana":
		return 2
	case "cherry":
		return 3
	case:
		return 99
	}
}

do_test :: proc() {
	g_r = classify16("banana")
}

main :: proc() {
	do_test()
	fmt.println("g_r =", g_r) // expect 2
	if g_r != 2 {
		fmt.println("FAIL")
	}
}
