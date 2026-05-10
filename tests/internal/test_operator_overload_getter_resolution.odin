package test_internal

import "core:testing"

Getter_Resolve :: struct {
	data:      [4]int,
	len:       int,
	value_hits: int,
	ptr_hits:   int,
}

@(operator="[]")
getter_resolve_value :: proc(v: ^Getter_Resolve, idx: int, loc := #caller_location) -> (value: int, ok: bool) #optional_ok {
	_ = loc
	v.value_hits += 1
	if idx < 0 || idx >= v.len {
		return 0, false
	}
	return v.data[idx], true
}

@(operator="[]")
getter_resolve_ptr :: proc(v: ^Getter_Resolve, idx: int, loc := #caller_location) -> (value: ^int, ok: bool) {
	_ = loc
	v.ptr_hits += 1
	if idx < 0 || idx >= v.len {
		return nil, false
	}
	return &v.data[idx], true
}

Getter_Resolve_Generic :: struct($E: typeid) {
	data:       [4]E,
	len:        int,
	value_hits: int,
	ptr_hits:   int,
}

@(operator="[]")
getter_resolve_generic_value :: proc(v: ^Getter_Resolve_Generic($E), idx: int, loc := #caller_location) -> (value: E, ok: bool) #optional_ok {
	_ = loc
	v.value_hits += 1
	if idx < 0 || idx >= v.len {
		return E{}, false
	}
	return v.data[idx], true
}

@(operator="[]")
getter_resolve_generic_ptr :: proc(v: ^Getter_Resolve_Generic($E), idx: int, loc := #caller_location) -> (value: ^E, ok: bool) {
	_ = loc
	v.ptr_hits += 1
	if idx < 0 || idx >= v.len {
		return nil, false
	}
	return &v.data[idx], true
}

@test
test_operator_overload_getter_resolution_value_vs_pointer_intent :: proc(t: ^testing.T) {
	c := Getter_Resolve{
		data = [4]int{10, 20, 30, 40},
		len  = 3,
	}

	// Single-result read must select value getter and unwrap optional-ok payload.
	v1 := c[1]
	testing.expect_value(t, v1, 20)
	testing.expect_value(t, c.value_hits, 1)
	testing.expect_value(t, c.ptr_hits, 0)

	// Two-result read must still select value getter.
	v2, ok2 := c[2]
	testing.expect_value(t, ok2, true)
	testing.expect_value(t, v2, 30)
	testing.expect_value(t, c.value_hits, 2)
	testing.expect_value(t, c.ptr_hits, 0)

	_, ok_missing := c[9]
	testing.expect_value(t, ok_missing, false)
	testing.expect_value(t, c.value_hits, 3)
	testing.expect_value(t, c.ptr_hits, 0)

	// Addressed read must select pointer getter.
	p1 := &c[0]
	testing.expect(t, p1 != nil)
	testing.expect_value(t, p1^, 10)
	testing.expect_value(t, c.value_hits, 3)
	testing.expect_value(t, c.ptr_hits, 1)

	p2, okp2 := &c[9]
	testing.expect_value(t, okp2, false)
	testing.expect_value(t, p2, nil)
	testing.expect_value(t, c.value_hits, 3)
	testing.expect_value(t, c.ptr_hits, 2)

	p3 := &c[2]
	testing.expect(t, p3 != nil)
	testing.expect_value(t, p3^, 30)
	testing.expect_value(t, c.value_hits, 3)
	testing.expect_value(t, c.ptr_hits, 3)

	p3^ = 77
	testing.expect_value(t, c.data[2], 77)
}

@test
test_operator_overload_getter_resolution_pointer_container :: proc(t: ^testing.T) {
	c := Getter_Resolve{
		data = [4]int{1, 2, 3, 4},
		len  = 4,
	}
	cp := &c

	// Value intent on pointer container still selects value getter.
	v, ok := cp[1]
	testing.expect_value(t, ok, true)
	testing.expect_value(t, v, 2)
	testing.expect_value(t, c.value_hits, 1)
	testing.expect_value(t, c.ptr_hits, 0)

	// Pointer intent on pointer container must work without taking ^^container.
	p := &cp[2]
	testing.expect(t, p != nil)
	testing.expect_value(t, p^, 3)
	testing.expect_value(t, c.value_hits, 1)
	testing.expect_value(t, c.ptr_hits, 1)
}

@test
test_operator_overload_getter_resolution_polymorphic_result_kind :: proc(t: ^testing.T) {
	c := Getter_Resolve_Generic(int){
		data = [4]int{11, 22, 33, 44},
		len  = 3,
	}

	// Exercises deferred polymorphic return type resolution for value getter.
	v, ok := c[1]
	testing.expect_value(t, ok, true)
	testing.expect_value(t, v, 22)
	testing.expect_value(t, c.value_hits, 1)
	testing.expect_value(t, c.ptr_hits, 0)

	// Exercises deferred polymorphic return type resolution for pointer getter.
	p, okp := &c[0]
	testing.expect_value(t, okp, true)
	testing.expect(t, p != nil)
	testing.expect_value(t, p^, 11)
	testing.expect_value(t, c.value_hits, 1)
	testing.expect_value(t, c.ptr_hits, 1)

	p^ = 99
	testing.expect_value(t, c.data[0], 99)
}
