// Test: indirect calls (proc values, function pointers)
//
// Fast-backend previously rejected indirect calls (calls through
// proc-typed values) at the call-info stage. Added support:
//   - fast_backend_get_call_info now accepts a proc-typed callee
//     expression when there's no Procedure entity
//   - fast_backend_emit_call_expr_internal materializes the function
//     pointer into work_reg, pushes it to spill before the args (so
//     the function pointer is at the bottom and gets popped last
//     into tmp_reg), and emits `blr tmp_reg` (arm64) / `call tmp_reg`
//     (x86-64) instead of `bl <symbol>` / `call <symbol>`
package test_proc_value

add :: proc(a, b: int) -> int {
	return a + b
}

add_c :: proc "c" (a, b: int) -> int {
	return a + b
}

g_r_odin:     int
g_r_cdecl:    int

g_p_odin:  proc(a, b: int) -> int = add
g_p_cdecl: proc "c" (a, b: int) -> int = add_c

do_test :: proc() {
	g_r_odin  = g_p_odin(10, 32)
	g_r_cdecl = g_p_cdecl(20, 22)
}

main :: proc() {
	do_test()
	// assert via runtime trap on failure
	if g_r_odin  != 42 {
		intrinsics.trap()
	}
	if g_r_cdecl != 42 {
		intrinsics.trap()
	}
}

import "base:intrinsics"
