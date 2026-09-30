package gfx

import "core:c"

Range :: struct {
	ptr: rawptr,
	size: c.size_t,
}

expected_ptr: rawptr

set_expected_ptr :: proc(value: rawptr) {
	expected_ptr = value
}

apply_byval :: proc(#any_int ub_slot: c.int, data: Range) {
	assert(ub_slot == 0)
	assert(data.ptr == expected_ptr)
	assert(data.size == 80)
}

apply_by_ptr :: proc(#any_int ub_slot: c.int, #by_ptr data: Range) {
	assert(ub_slot == 0)
	assert(data.ptr == expected_ptr)
	assert(data.size == 80)
	apply_byval(ub_slot, data)
}
