package test_internal

import queue "core:container/queue"
import small_array "core:container/small_array"
import big "core:math/big"
import fixed "core:math/fixed"
import "core:testing"
import utf8string "core:unicode/utf8/utf8string"

Wrap_Int :: struct {
	a: int,
}

@(operator="+")
wrap_int_add :: proc(lhs, rhs: Wrap_Int) -> Wrap_Int {
	return {lhs.a + rhs.a}
}

Wrap_Vec4 :: struct {
	v: [4]f32,
}

@(operator="+")
wrap_vec4_add :: proc(lhs, rhs: Wrap_Vec4) -> Wrap_Vec4 {
	return {lhs.v + rhs.v}
}

Wrap_Index :: struct {
	data: [4]int,
}

Wrap_Iter :: struct {
	data: []int,
}

Wrap_Iter_Dual :: struct {
	data: []int,
}

Wrap_Generic_Num :: struct($E: typeid) {
	a: E,
}

Wrap_Generic_Index :: struct($E: typeid) {
	data: [4]E,
}

Wrap_Index_Map_Semantics :: struct {
	data: [4]int,
	hits: int,
}

Wrap_Loc_Binary :: struct {
	v: int,
}

Wrap_Loc_Index :: struct {
	data: [4]int,
}

wrap_loc_binary_last_line: i32
wrap_loc_index_get_last_line: i32
wrap_loc_index_set_last_line: i32

@(operator="[]")
wrap_index_get :: proc(v: Wrap_Index, idx: int) -> int {
	return v.data[idx]
}

@(operator="[]=")
wrap_index_set :: proc(v: ^Wrap_Index, idx: int, value: int) {
	v.data[idx] = value
}

@(operator="+")
wrap_generic_num_add :: proc(lhs, rhs: Wrap_Generic_Num($E)) -> Wrap_Generic_Num(E) {
	return {lhs.a + rhs.a}
}

@(operator="[]")
wrap_generic_index_get :: proc(v: Wrap_Generic_Index($E), idx: int) -> E {
	return v.data[idx]
}

@(operator="[]=")
wrap_generic_index_set :: proc(v: ^Wrap_Generic_Index($E), idx: int, value: E) {
	v.data[idx] = value
}

@(operator="[]")
wrap_index_map_semantics_peek :: proc(v: Wrap_Index_Map_Semantics, idx: int) -> (value: int, ok: bool) {
	if idx < 0 || idx >= len(v.data) {
		return 0, false
	}
	return v.data[idx], true
}

@(operator="[]")
wrap_index_map_semantics_get_ptr :: proc(v: ^Wrap_Index_Map_Semantics, idx: int) -> (value: ^int, ok: bool) {
	if idx < 0 || idx >= len(v.data) {
		return nil, false
	}
	v.hits += 1
	return &v.data[idx], true
}

@(operator="+")
wrap_loc_binary_add :: proc(a, b: Wrap_Loc_Binary, loc := #caller_location) -> Wrap_Loc_Binary {
	wrap_loc_binary_last_line = loc.line
	return {v = a.v + b.v}
}

@(operator="[]")
wrap_loc_index_get :: proc(v: Wrap_Loc_Index, idx: int, loc := #caller_location) -> int {
	wrap_loc_index_get_last_line = loc.line
	return v.data[idx]
}

@(operator="[]=")
wrap_loc_index_set :: proc(v: ^Wrap_Loc_Index, idx: int, value: int, loc := #caller_location) {
	wrap_loc_index_set_last_line = loc.line
	v.data[idx] = value
}

@(operator="in")
wrap_iter_next :: proc(w: Wrap_Iter, it: ^int) -> (value: int, index: int, ok: bool) {
	if it^ >= len(w.data) {
		return 0, 0, false
	}
	index = it^
	value = w.data[index]
	it^ += 1
	return value, index, true
}

@(operator="in")
wrap_iter_dual_next_value :: proc(w: Wrap_Iter_Dual, it: ^int) -> (value: int, ok: bool) {
	if it^ >= len(w.data) {
		return 0, false
	}
	value = w.data[it^] * 10
	it^ += 1
	return value, true
}

@(operator="in")
wrap_iter_dual_next_indexed :: proc(w: Wrap_Iter_Dual, it: ^int) -> (value: int, index: int, ok: bool) {
	if it^ >= len(w.data) {
		return 0, 0, false
	}
	index = it^
	value = w.data[index]
	it^ += 1
	return value, index, true
}

@test
test_operator_overload_add_for_struct_wrappers :: proc(t: ^testing.T) {
	a := Wrap_Int{1}
	b := Wrap_Int{3}
	c := a + b
	testing.expect_value(t, c.a, 4)

	x := Wrap_Vec4{[4]f32{1, 2, 3, 4}}
	y := Wrap_Vec4{[4]f32{5, 6, 7, 8}}
	z := x + y
	testing.expect_value(t, z.v, [4]f32{6, 8, 10, 12})
}

@test
test_operator_overload_keeps_default_eq_ne_rules :: proc(t: ^testing.T) {
	a := Wrap_Int{7}
	b := Wrap_Int{7}
	c := Wrap_Int{8}

	testing.expect_value(t, a == b, true)
	testing.expect_value(t, a != b, false)
	testing.expect_value(t, a == c, false)
	testing.expect_value(t, a != c, true)
}

@test
test_operator_overload_index_get_set :: proc(t: ^testing.T) {
	v := Wrap_Index{[4]int{1, 2, 3, 4}}

	testing.expect_value(t, v[2], 3)
	v[2] = 99
	testing.expect_value(t, v[2], 99)
}

@test
test_operator_overload_polymorphic_add :: proc(t: ^testing.T) {
	a := Wrap_Generic_Num(int){11}
	b := Wrap_Generic_Num(int){31}
	c := a + b
	testing.expect_value(t, c.a, 42)
}

@test
test_operator_overload_polymorphic_index_get_set :: proc(t: ^testing.T) {
	v := Wrap_Generic_Index(int){[4]int{2, 4, 6, 8}}
	testing.expect_value(t, v[1], 4)
	v[1] = 123
	testing.expect_value(t, v[1], 123)
}

@test
test_operator_overload_index_address_prefers_pointer_receiver :: proc(t: ^testing.T) {
	v := Wrap_Index_Map_Semantics{data = [4]int{10, 20, 30, 40}}

	peek, ok := v[2]
	testing.expect_value(t, ok, true)
	testing.expect_value(t, peek, 30)
	testing.expect_value(t, v.hits, 0)

	ptr, ok2 := &v[2]
	testing.expect_value(t, ok2, true)
	testing.expect(t, ptr != nil)
	testing.expect_value(t, ptr^, 30)
	testing.expect_value(t, v.hits, 1)

	only_ptr := &v[2]
	testing.expect(t, only_ptr != nil)
	testing.expect_value(t, only_ptr^, 30)
	testing.expect_value(t, v.hits, 2)

	ptr^ = 99
	testing.expect_value(t, v.data[2], 99)
}

@test
test_operator_overload_supports_caller_location_defaults :: proc(t: ^testing.T) {
	wrap_loc_binary_last_line = 0
	wrap_loc_index_get_last_line = 0
	wrap_loc_index_set_last_line = 0

	a := Wrap_Loc_Binary{v = 10}
	b := Wrap_Loc_Binary{v = 32}
	c := a + b
	testing.expect_value(t, c.v, 42)
	testing.expect(t, wrap_loc_binary_last_line > 0)

	idx := Wrap_Loc_Index{data = [4]int{1, 2, 3, 4}}
	_ = idx[1]
	testing.expect(t, wrap_loc_index_get_last_line > 0)

	idx[2] = 99
	testing.expect(t, wrap_loc_index_set_last_line > 0)
	testing.expect_value(t, idx.data[2], 99)
}

@test
test_operator_overload_core_fixed_math :: proc(t: ^testing.T) {
	a := fixed.Fixed16_16{i = 3 << 16}
	b := fixed.Fixed16_16{i = 1 << 15}

	sum := a + b
	diff := a - b
	prod := a * b
	quo := a / b

	testing.expect_value(t, fixed.to_f64(sum), 3.5)
	testing.expect_value(t, fixed.to_f64(diff), 2.5)
	testing.expect_value(t, fixed.to_f64(prod), 1.5)
	testing.expect_value(t, fixed.to_f64(quo), 6.0)
}

@test
test_operator_overload_core_queue_index :: proc(t: ^testing.T) {
	q: queue.Queue(int)
	err := queue.init(&q)
	testing.expect(t, err == nil)
	defer queue.destroy(&q)

	_, err = queue.push_back(&q, 11)
	testing.expect(t, err == nil)
	_, err = queue.push_back(&q, 22)
	testing.expect(t, err == nil)

	testing.expect_value(t, q[0], 11)
	testing.expect_value(t, q[1], 22)

	q[1] = 77
	testing.expect_value(t, q[1], 77)
}

@test
test_operator_overload_core_small_array_index :: proc(t: ^testing.T) {
	a: small_array.Small_Array(4, int)
	a.data = [4]int{5, 6, 7, 0}
	a.len = 3

	testing.expect_value(t, a[1], 6)
	a[1] = 42
	testing.expect_value(t, a[1], 42)
}

@test
test_operator_overload_core_utf8string_index :: proc(t: ^testing.T) {
	s: utf8string.String
	utf8string.init(&s, "ab\u00e9")

	testing.expect_value(t, s[0], rune('a'))
	testing.expect_value(t, s[2], rune(0x00e9))
}

@test
test_operator_overload_core_big_math :: proc(t: ^testing.T) {
	a, b: big.Int
	testing.expect(t, big.set(&a, 40) == nil)
	testing.expect(t, big.set(&b, 2) == nil)

	sum := a + b
	diff := a - b
	prod := a * b
	quo := a / b
	defer big.destroy(&a, &b, &sum, &diff, &prod, &quo)

	sum_v, err := big.get(&sum, i64)
	testing.expect(t, err == nil)
	diff_v, err2 := big.get(&diff, i64)
	testing.expect(t, err2 == nil)
	prod_v, err3 := big.get(&prod, i64)
	testing.expect(t, err3 == nil)
	quo_v, err4 := big.get(&quo, i64)
	testing.expect(t, err4 == nil)

	testing.expect_value(t, sum_v, i64(42))
	testing.expect_value(t, diff_v, i64(38))
	testing.expect_value(t, prod_v, i64(80))
	testing.expect_value(t, quo_v, i64(20))

	r1, r2: big.Rat
	testing.expect(t, big.set(&r1, f64(1.5)) == nil)
	testing.expect(t, big.set(&r2, f64(0.5)) == nil)

	r_sum := r1 + r2
	r_mixed := a + r2
	defer big.destroy(&r1, &r2, &r_sum, &r_mixed)

	r_sum_f, r_sum_exact, rat_err := big.rat_to_f64(&r_sum)
	testing.expect(t, rat_err == nil)
	testing.expect_value(t, r_sum_exact, true)
	testing.expect_value(t, r_sum_f, f64(2.0))

	r_mixed_f, _, rat_err2 := big.rat_to_f64(&r_mixed)
	testing.expect(t, rat_err2 == nil)
	testing.expect_value(t, r_mixed_f, f64(40.5))
}

@test
test_operator_overload_iterator_for_in :: proc(t: ^testing.T) {
	w := Wrap_Iter{[]int{3, 5, 7}}

	sum := 0
	for v in w {
		sum += v
	}
	testing.expect_value(t, sum, 15)

	weighted := 0
	for v, i in w {
		weighted += v * (i + 1)
	}
	testing.expect_value(t, weighted, 34)

	counter := 0
	total := 0
	for counter = 100; v in w {
		total += v
		counter += 1
	}
	testing.expect_value(t, total, 15)
	testing.expect_value(t, counter, 103)
}

@test
test_operator_overload_iterator_for_in_arity_selection :: proc(t: ^testing.T) {
	w := Wrap_Iter_Dual{[]int{2, 4, 6}}

	// Single-value loop must pick the 2-result overload: (value, ok).
	sum_value_only := 0
	for v in w {
		sum_value_only += v
	}
	testing.expect_value(t, sum_value_only, 120)

	// Two-value loop must pick the 3-result overload: (value, index, ok).
	sum_with_index := 0
	for v, i in w {
		sum_with_index += v * (i + 1)
	}
	testing.expect_value(t, sum_with_index, 28)
}
