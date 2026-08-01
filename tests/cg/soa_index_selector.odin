package main

import "core:reflect"

Probe :: struct {
	value: i32,
}

field_offset :: proc(id: typeid, index: int) -> uintptr {
	fields := reflect.struct_fields_zipped(id)
	return fields[index].offset
}

main :: proc() {
	_ = field_offset(Probe, 0)
}
