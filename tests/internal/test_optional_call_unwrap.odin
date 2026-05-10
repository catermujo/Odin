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

optional_multi_value :: proc(ok: bool) -> (a, b, c, d: int, found: bool) #optional_ok {
	if !ok {
		return 0, 0, 0, 0, false
	}
	return 1, 2, 3, 4, true
}

optional_alloc_multi_value :: proc(ok: bool) -> (a, b: int, err: runtime.Allocator_Error) #optional_allocator_error {
	if !ok {
		return 0, 0, .Out_Of_Memory
	}
	return 11, 13, .None
}

optional_multi_forward :: proc(ok: bool) -> (a, b, c, d: int, found: bool) #optional_ok {
	return optional_multi_value(ok)
}

optional_alloc_multi_forward :: proc(ok: bool) -> (a, b: int, err: runtime.Allocator_Error) #optional_allocator_error {
	return optional_alloc_multi_value(ok)
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

@test
test_optional_ok_multi_result_unwrap_success :: proc(t: ^testing.T) {
	a, b, c, d := optional_multi_value(true)
	testing.expect_value(t, a, 1)
	testing.expect_value(t, b, 2)
	testing.expect_value(t, c, 3)
	testing.expect_value(t, d, 4)
}

@test
test_optional_ok_multi_result_unwrap_assert :: proc(t: ^testing.T) {
	testing.expect_assert(t, "Invalid optional value")
	_, _, _, _ = optional_multi_value(false)
}

@test
test_optional_ok_multi_result_explicit_tag :: proc(t: ^testing.T) {
	a, b, c, d, ok := optional_multi_value(false)
	testing.expect_value(t, a, 0)
	testing.expect_value(t, b, 0)
	testing.expect_value(t, c, 0)
	testing.expect_value(t, d, 0)
	testing.expect_value(t, ok, false)
}

@test
test_optional_allocator_error_multi_result_unwrap_success :: proc(t: ^testing.T) {
	a, b := optional_alloc_multi_value(true)
	testing.expect_value(t, a, 11)
	testing.expect_value(t, b, 13)
}

@test
test_optional_allocator_error_multi_result_unwrap_assert :: proc(t: ^testing.T) {
	testing.expect_assert(t, "Invalid optional value")
	_, _ = optional_alloc_multi_value(false)
}

@test
test_optional_allocator_error_multi_result_explicit_tag :: proc(t: ^testing.T) {
	a, b, err := optional_alloc_multi_value(false)
	testing.expect_value(t, a, 0)
	testing.expect_value(t, b, 0)
	testing.expect_value(t, err, runtime.Allocator_Error.Out_Of_Memory)
}

@test
test_optional_ok_multi_result_forward_explicit_tag :: proc(t: ^testing.T) {
	_, _, _, _, ok := optional_multi_forward(false)
	testing.expect_value(t, ok, false)
}

@test
test_optional_allocator_error_multi_result_forward_explicit_tag :: proc(t: ^testing.T) {
	_, _, err := optional_alloc_multi_forward(false)
	testing.expect_value(t, err, runtime.Allocator_Error.Out_Of_Memory)
}
