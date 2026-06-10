package test_internal

import "core:testing"

switch_tag_scalar :: proc() -> int {
	return 2
}

switch_case_scalar :: proc(value: int) -> int {
	return value
}

@test
test_switch_scalar_tag_calls :: proc(t: ^testing.T) {
	matched := 0
	switch switch_tag_scalar() {
	case switch_case_scalar(1):
	case switch_case_scalar(2):
		matched = 2
	case:
		matched = -1
	}
	testing.expect_value(t, matched, 2)
}
