package test_internal

import "core:testing"

Fallible_Index :: struct {
	data: [4]int,
	len:  int,
}

@(operator="[]")
fallible_index_get :: proc(v: Fallible_Index, idx: int) -> (value: int, ok: bool) {
	if idx < 0 || idx >= v.len {
		return 0, false
	}
	return v.data[idx], true
}

double_from_index_or_return :: proc(v: Fallible_Index, idx: int) -> (res: int, ok: bool) {
	value := (v[idx]) or_return
	return value * 2, true
}

@test
test_operator_overload_fallible_index_get :: proc(t: ^testing.T) {
	v := Fallible_Index{
		data = [4]int{10, 20, 30, 40},
		len  = 3,
	}

	value, ok := v[1]
	testing.expect_value(t, value, 20)
	testing.expect_value(t, ok, true)

	_, ok = v[3]
	testing.expect_value(t, ok, false)

	fallback := (v[3]) or_else -1
	testing.expect_value(t, fallback, -1)

	doubled, ok2 := double_from_index_or_return(v, 1)
	testing.expect_value(t, doubled, 40)
	testing.expect_value(t, ok2, true)

	_, ok3 := double_from_index_or_return(v, 3)
	testing.expect_value(t, ok3, false)
}
