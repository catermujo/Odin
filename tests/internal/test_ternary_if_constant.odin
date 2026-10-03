#+test
package test_internal

import "core:testing"

TERNARY_IF_CONFIG_DEFAULT :: #config(TERNARY_IF_CONFIG, 62 if ODIN_OS == .Darwin else 63)

@(test)
ternary_if_constant_folding :: proc(t: ^testing.T) {
	TRUE_VALUE :: 13 if true else 29
	FALSE_VALUE :: 13 if false else 29
	NESTED_VALUE :: (13 if false else 29) if true else 41
	FLOAT_VALUE: f64 : 1.5 if false else 2.5
	STRING_VALUE :: "left" if false else "right"
	ENUM_VALUE :: ODIN_OS if true else ODIN_OS
	ARRAY_VALUE :: [3]int{1, 2, 3} if false else [3]int{4, 5, 6}
	#assert(TRUE_VALUE == 13)
	#assert(FALSE_VALUE == 29)
	#assert(NESTED_VALUE == 29)
	#assert(FLOAT_VALUE == 2.5)
	#assert(STRING_VALUE == "right")
	#assert(ENUM_VALUE == ODIN_OS)
	#assert(ARRAY_VALUE[0] == 4 && ARRAY_VALUE[2] == 6)
	#assert((62 if ODIN_OS == .Darwin else 63) == (62 when ODIN_OS == .Darwin else 63))
	testing.expect_value(t, TERNARY_IF_CONFIG_DEFAULT, #config(TERNARY_IF_CONFIG, 62 when ODIN_OS == .Darwin else 63))
	left, right := 13, 29
	condition := true
	testing.expect_value(t, TRUE_VALUE, left if condition else right)
	condition = false
	testing.expect_value(t, FALSE_VALUE, left if condition else right)
}

@(test)
ternary_if_runtime_branch_selection :: proc(t: ^testing.T) {
	calls := 0
	branch := proc(calls: ^int, value: int) -> int {
		calls^ += 1
		return value
	}
	testing.expect_value(t, branch(&calls, 13) if true else branch(&calls, 29), 13)
	testing.expect_value(t, calls, 1)
	testing.expect_value(t, branch(&calls, 13) if false else branch(&calls, 29), 29)
	testing.expect_value(t, calls, 2)
	testing.expect_value(t, 13 if true else branch(&calls, 29), 13)
	testing.expect_value(t, branch(&calls, 13) if false else 29, 29)
	testing.expect_value(t, calls, 2)
}
