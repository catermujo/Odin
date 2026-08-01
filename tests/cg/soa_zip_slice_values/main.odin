package main

import "core:reflect"

Probe :: struct {
    label: string `json:"label"`,
    value: i32,
}

make_inputs :: #force_no_inline proc() -> (names: []string, values: []int) {
    names = make([]string, 4)
    values = make([]int, 5)
    names[0] = "zero"
    names[1] = "one"
    names[2] = "two"
    names[3] = "three"
    for i in 0 ..< len(values) {
        values[i] = 10 + i
    }
    return
}

main :: proc() {
    names, values := make_inputs()
    address_backed := names[1:]
    named := soa_zip(name = address_backed, value = values[:2])

    assert(len(named) == 2)
    assert(rawptr(named.name) == raw_data(address_backed))
    assert(rawptr(named.value) == raw_data(values))
    assert(named.name[0] == "one")
    assert(named.name[1] == "two")
    assert(named.value[0] == 10)
    assert(named.value[1] == 11)

    named_total := 0
    for field in named {
        named_total += len(field.name) + field.value
    }
    assert(named_total == 27)

    positional := soa_zip(names[1:], values[:2])
    positional_names, positional_values := soa_unzip(positional)
    assert(len(positional_names) == 2)
    assert(raw_data(positional_names) == raw_data(address_backed))
    assert(raw_data(positional_values) == raw_data(values))
    assert(positional_names[0] == "one")
    assert(positional_values[1] == 11)

    fields := reflect.struct_fields_zipped(Probe)
    assert(len(fields) == 2)
    assert(fields.name[0] == "label")
    assert(fields.name[1] == "value")
    assert(reflect.are_types_identical(fields.type[0], type_info_of(string)))
    assert(reflect.are_types_identical(fields.type[1], type_info_of(i32)))
    assert(fields.offset[0] == 0)
    assert(fields.offset[1] > fields.offset[0])
    assert(fields.tag[0] == reflect.Struct_Tag(`json:"label"`))
    assert(fields.tag[1] == "")
    assert(!fields.is_using[0])
    assert(!fields.is_using[1])
}
