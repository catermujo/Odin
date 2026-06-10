package test_internal

import "base:intrinsics"
import "base:runtime"
import "core:testing"

probe_default_args :: proc(value, expected: $T, loc := #caller_location, value_expr := #caller_expression(value)) -> (bool, runtime.Source_Code_Location, string) where intrinsics.type_is_comparable(T) {
	return value == expected, loc, value_expr
}

@test
test_polymorphic_default_args :: proc(t: ^testing.T) {
	ok, loc, value_expr := probe_default_args(123, 123)
	testing.expect_value(t, ok, true)
	testing.expect_value(t, value_expr, "123")
	testing.expect_value(t, loc.procedure, "test_polymorphic_default_args")
	testing.expect_value(t, loc.line > 0, true)
	testing.expect_value(t, loc.column > 0, true)
}
