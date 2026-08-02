package main

import "core:testing"
import "core:c/libc"

comparison_branch :: proc(a, b: int) -> int {
	if a < b {
		return 7
	}
	return 9
}

comparison_stored_and_returned :: proc(a, b: int) -> bool {
	result := a < b
	return result
}

logical_temporary :: proc(a, b, c: int) -> bool {
	return (a < b && b < c) || a == c
}

not_temporary :: proc(a, b: int) -> bool {
	return !(a < b)
}

nonzero_bool_is_true :: proc() -> bool {
	value := transmute(bool)u8(2)
	return value && !(!value)
}

any_bool_round_trip :: proc() -> bool {
	value: any = 1 < 2
	result, ok := value.(bool)
	return ok && result
}

any_runtime_comparison_round_trip :: proc(a, b: int) -> bool {
	value: any = a < b
	result, ok := value.(bool)
	return ok && result
}

any_constant_logical_round_trip :: proc(flag: bool) -> bool {
	left: any = false && flag
	right: any = true || flag
	left_result, left_ok := left.(bool)
	right_result, right_ok := right.(bool)
	return left_ok && right_ok && !left_result && right_result
}

c_vararg_bool :: proc() {
	_ = libc.printf("", 1 < 2)
}

@(test)
legacy_bool_temporaries_test :: proc(t: ^testing.T) {
	_ = t
	assert(comparison_branch(2, 3) == 7)
	assert(comparison_branch(3, 2) == 9)
	assert(comparison_stored_and_returned(2, 3))
	assert(!comparison_stored_and_returned(3, 2))
	assert(logical_temporary(2, 3, 4))
	assert(logical_temporary(2, 3, 2))
	assert(!logical_temporary(4, 3, 2))
	assert(not_temporary(3, 2))
	assert(!not_temporary(2, 3))
	assert(nonzero_bool_is_true())
	assert(any_bool_round_trip())
	assert(any_runtime_comparison_round_trip(2, 3))
	assert(any_constant_logical_round_trip(false))
	c_vararg_bool()
}

main :: proc() {
	legacy_bool_temporaries_test(nil)
}
