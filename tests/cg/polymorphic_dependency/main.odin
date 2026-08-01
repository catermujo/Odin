package main

import "core:reflect"

inner :: proc($T: typeid, value: T) -> T {
    return value
}

outer :: proc($T: typeid, value: T) -> T {
    return inner(T, value)
}

generic_map_capacity :: proc($K: typeid, $V: typeid) -> int {
    m := make(map[K]V, 4)
    return cap(m)
}

generic_struct_field_count :: proc($T: typeid) -> int {
    fields := reflect.struct_fields_zipped(T)
    return len(fields)
}

Test_Struct :: struct {
    value: i32,
}

Test_Enum :: enum {
    one,
    two,
}

generic_enum_field_count :: proc($T: typeid) -> int {
    count: int
    for &field in reflect.enum_fields_zipped(T) {
        count += 1
        _ = field
    }
    return count
}

zero_sized_result :: proc() -> (value: i32, ignored: struct{}) {
    value = 7
    return
}

main :: proc() {
    assert(outer(i32, 42) == 42)
    assert(generic_map_capacity(int, string) == 8)
    assert(generic_struct_field_count(Test_Struct) == 1)
    assert(generic_enum_field_count(Test_Enum) == 2)
    value, _ := zero_sized_result()
    assert(value == 7)
}
