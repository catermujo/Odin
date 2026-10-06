#+test
package test_internal

import "core:testing"

@(test)
union_assignment_preserves_nested_literal_variants :: proc(t: ^testing.T) {
	Inner :: struct {
		text: union {string, int},
		number: union {i8, i16},
	}
	Outer :: union {Inner, bool}
	Request :: struct {line: Outer}

	request: Request
	request.line = Inner {text = "payload", number = i16(300)}
	if inner, ok := request.line.(Inner); testing.expect(t, ok, "outer struct variant lost") {
		if text, text_ok := inner.text.(string); testing.expect(t, text_ok, "inner string variant lost") {
			testing.expect_value(t, text, "payload")
		}
		if number, number_ok := inner.number.(i16); testing.expect(t, number_ok, "inner i16 variant lost") {
			testing.expect_value(t, number, i16(300))
		}
	}
}

@(test)
union_initialization_and_return_preserve_nested_literals :: proc(t: ^testing.T) {
	Inner :: struct {value: union {string, int}}
	Outer :: union {Inner, bool}
	make_value :: proc() -> Outer {
		return Inner {value = "returned"}
	}

	initialized: Outer = Inner {value = 42}
	if inner, ok := initialized.(Inner); testing.expect(t, ok, "initialized struct variant lost") {
		if number, number_ok := inner.value.(int); testing.expect(t, number_ok, "initialized int variant lost") {
			testing.expect_value(t, number, 42)
		}
	}
	returned := make_value()
	if inner, ok := returned.(Inner); testing.expect(t, ok, "returned struct variant lost") {
		if text, text_ok := inner.value.(string); testing.expect(t, text_ok, "returned string variant lost") {
			testing.expect_value(t, text, "returned")
		}
	}
}

@(test)
union_rechecking_preserves_nested_union_casts :: proc(t: ^testing.T) {
	Scalar :: union {int, bool}
	Nested :: union {Scalar, string}
	Inner :: struct {value: Nested}
	Outer :: union {Inner, f64}

	outer: Outer = Inner {value = Nested(Scalar(7))}
	if inner, ok := outer.(Inner); testing.expect(t, ok, "outer struct variant lost") {
		if scalar, scalar_ok := inner.value.(Scalar); testing.expect(t, scalar_ok, "nested scalar variant lost") {
			if number, number_ok := scalar.(int); testing.expect(t, number_ok, "scalar int variant lost") {
				testing.expect_value(t, number, 7)
			}
		}
	}
}
