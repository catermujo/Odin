package test_internal

import "base:runtime"
import "core:testing"

optional_int_value :: proc(ok: bool) -> (value: int, found: bool) #optional_ok {
	if !ok {
		return 0, false
	}
	return 42, true
}

optional_alloc_value :: proc(ok: bool) -> (value: int, err: runtime.Allocator_Error) #optional_allocator_error {
	if !ok {
		return 0, .Out_Of_Memory
	}
	return 7, .None
}

@test
test_optional_ok_single_result_unwrap_success :: proc(t: ^testing.T) {
	v := optional_int_value(true)
	testing.expect_value(t, v, 42)
}

@test
test_optional_ok_single_result_unwrap_assert :: proc(t: ^testing.T) {
	testing.expect_assert(t, "Invalid optional value")
	_ = optional_int_value(false)
}

@test
test_optional_allocator_error_single_result_unwrap_success :: proc(t: ^testing.T) {
	v := optional_alloc_value(true)
	testing.expect_value(t, v, 7)
}

@test
test_optional_allocator_error_single_result_unwrap_assert :: proc(t: ^testing.T) {
	testing.expect_assert(t, "Invalid optional value")
	_ = optional_alloc_value(false)
}
