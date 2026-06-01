package test_internal

import "core:testing"

// ---- type / size checks ----------------------------------------------------

@(test)
lambda_type_size :: proc(t: ^testing.T) {
	// a closure value is a 2-word fat pointer {fn_ptr, env_ptr}
	testing.expect_value(t, size_of(lambda() -> int), 2 * size_of(rawptr))
	// a plain proc stays a single pointer
	testing.expect_value(t, size_of(proc() -> int), size_of(rawptr))
	// different signatures must have the same value size
	testing.expect_value(t, size_of(lambda(int, int) -> int), size_of(lambda() -> bool))
}

// ---- by-value capture ------------------------------------------------------

@(test)
lambda_capture_by_value :: proc(t: ^testing.T) {
	a := 5
	add := lambda [a](x: int) -> int { return a + x }

	// the closure holds a frozen snapshot taken at creation time
	a = 100
	testing.expect_value(t, add(1), 6) // 5+1, NOT 100+1
}

@(test)
lambda_capture_by_value_multiple :: proc(t: ^testing.T) {
	x := 10
	y := 20
	f := lambda [x, y]() -> int { return x + y }

	x = 0
	y = 0
	testing.expect_value(t, f(), 30) // frozen copies
}

@(test)
lambda_by_value_returns_correct_result :: proc(t: ^testing.T) {
	mul := 3
	scale := lambda [mul](n: int) -> int { return mul * n }
	testing.expect_value(t, scale(7), 21)
	testing.expect_value(t, scale(0), 0)
}

// ---- by-reference capture --------------------------------------------------

@(test)
lambda_capture_by_ref_reads_live_value :: proc(t: ^testing.T) {
	counter := 0
	bump := lambda [&counter]() { counter += 1 }
	bump()
	bump()
	bump()
	testing.expect_value(t, counter, 3)
}

@(test)
lambda_capture_by_ref_write_visible_outside :: proc(t: ^testing.T) {
	value := 42
	set := lambda [&value](v: int) { value = v }
	set(99)
	testing.expect_value(t, value, 99)
}

@(test)
lambda_capture_by_ref_and_by_value_mixed :: proc(t: ^testing.T) {
	base   := 10
	offset := 1
	adder := lambda [base, &offset](n: int) -> int { return base + offset + n }

	// base is by value (frozen at 10), offset is by reference (live)
	base   = 999
	offset = 5
	testing.expect_value(t, adder(0), 15) // 10 + 5 + 0
}

// ---- empty-capture (no-environment) ----------------------------------------

@(test)
lambda_no_capture :: proc(t: ^testing.T) {
	sq := lambda [](n: int) -> int { return n * n }
	testing.expect_value(t, sq(4), 16)
	testing.expect_value(t, sq(0), 0)
}

// ---- struct field ----------------------------------------------------------

@(test)
lambda_in_struct_field :: proc(t: ^testing.T) {
	Transformer :: struct { f: lambda(int) -> int }

	h := Transformer{ f = lambda [](x: int) -> int { return x * x } }
	testing.expect_value(t, h.f(4), 16)
}

@(test)
lambda_in_struct_field_capturing :: proc(t: ^testing.T) {
	Predicate :: struct { check: lambda(int) -> bool }

	threshold := 10
	p := Predicate{ check = lambda [threshold](n: int) -> bool { return n > threshold } }
	testing.expect(t, p.check(11))
	testing.expect(t, !p.check(10))
}

// ---- direct field assignment vs compound literal ---------------------------

@(test)
lambda_struct_field_direct_assign :: proc(t: ^testing.T) {
	Holder :: struct { f: lambda(int) -> int }

	h: Holder
	base := 7
	h.f = lambda [base](x: int) -> int { return base + x }
	testing.expect_value(t, h.f(3), 10)
}

// ---- escaping via closure_clone / closure_free -----------------------------

// NOTE: an escaping counter must capture BY VALUE so the mutable state lives inside the environment
// (which closure_clone copies to the heap). Capturing the local by reference would dangle once
// make_counter returns — that is the documented by-reference escape hazard, not a counter.
make_counter :: proc(start: int) -> lambda() -> int {
	n := start
	inc := lambda [n]() -> int { n += 1; return n }
	return closure_clone(inc, context.allocator)
}

@(test)
lambda_closure_clone_escapes_frame :: proc(t: ^testing.T) {
	c := make_counter(0)
	defer closure_free(c, context.allocator)

	// the heap environment persists between calls, so the by-value copy keeps mutating
	testing.expect_value(t, c(), 1)
	testing.expect_value(t, c(), 2)
	testing.expect_value(t, c(), 3)
}

@(test)
lambda_closure_clone_by_value_is_independent :: proc(t: ^testing.T) {
	make_adder :: proc(base: int) -> lambda(int) -> int {
		f := lambda [base](x: int) -> int { return base + x }
		return closure_clone(f, context.allocator)
	}

	add5 := make_adder(5)
	add9 := make_adder(9)
	defer closure_free(add5, context.allocator)
	defer closure_free(add9, context.allocator)

	testing.expect_value(t, add5(1), 6)
	testing.expect_value(t, add9(1), 10)
}

// ---- passing lambdas as proc arguments -------------------------------------

apply :: proc(f: lambda(int) -> int, v: int) -> int {
	return f(v)
}

@(test)
lambda_passed_as_argument :: proc(t: ^testing.T) {
	bias := 3
	f := lambda [bias](x: int) -> int { return x + bias }
	testing.expect_value(t, apply(f, 4), 7)
}

// ---- struct layout with a closure field -----------------------------------

@(test)
lambda_struct_field_size :: proc(t: ^testing.T) {
	Box :: struct { tag: int, f: lambda(int) -> int }
	// the closure field is two pointers wide; layout must account for that (not a single pointer)
	testing.expect_value(t, size_of(Box), size_of(int) + 2 * size_of(rawptr))
}

// ---- array of closures -----------------------------------------------------

@(test)
lambda_array_of_closures :: proc(t: ^testing.T) {
	k := 100
	fns := [3]lambda(int) -> int{
		lambda [ ](x: int) -> int { return x },
		lambda [ ](x: int) -> int { return x * 2 },
		lambda [k](x: int) -> int { return x + k },
	}
	testing.expect_value(t, fns[0](5), 5)
	testing.expect_value(t, fns[1](5), 10)
	testing.expect_value(t, fns[2](5), 105)
}

// ---- type information ------------------------------------------------------

@(test)
lambda_typeid_distinct_from_proc :: proc(t: ^testing.T) {
	a := typeid_of(lambda(int) -> int)
	b := typeid_of(proc(int) -> int)
	testing.expect(t, a != b)                 // a closure type is never the same as a bare proc type
	testing.expect(t, type_info_of(a) != nil) // type-info emission for a closure must not crash
}

// ---- error cases: these must NOT compile, verified by the checker ----------
//
// The following are left as comments to document expected rejection:
//
//   lambda_bad_capture_undeclared :: proc() {
//       f := lambda [z]() {}  // z not declared -> error
//   }
//
//   lambda_bad_nocapture :: proc() {
//       x := 5
//       f := lambda [](n: int) -> int { return x + n }  // x not captured -> error
//   }
