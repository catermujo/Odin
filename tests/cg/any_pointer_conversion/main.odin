package main

import "core:reflect"

Pair :: struct {
    left:  int,
    right: f32,
}

calls: int

make_int_pointer :: #force_no_inline proc(value: int) -> ^int {
    calls += 1
    result := new(int)
    result^ = value
    return result
}

make_pair_pointer :: #force_no_inline proc(value: Pair) -> ^Pair {
    result := new(Pair)
    result^ = value
    return result
}

make_array_pointer :: #force_no_inline proc(value: [2]int) -> ^[2]int {
    result := new([2]int)
    result^ = value
    return result
}

make_pointer_pointer :: #force_no_inline proc(value: ^int) -> ^^int {
    result := new(^int)
    result^ = value
    return result
}

assert_pointer_box :: proc(boxed: any, pointer: rawptr, id: typeid) {
    assert(boxed.id == id)
    assert(boxed.data != nil)
    assert((^rawptr)(boxed.data)^ == pointer)
}

assert_deref :: proc(boxed: any, pointer: rawptr, id: typeid) -> any {
    dereferenced := reflect.deref(boxed)
    assert(dereferenced.id == id)
    assert(dereferenced.data == pointer)
    return dereferenced
}

take_any :: proc(value: any) -> any {
    return value
}

take_variadic :: proc(values: ..any) -> any {
    assert(len(values) == 1)
    return values[0]
}

main :: proc() {
    int_pointer := make_int_pointer(42)
    pair := Pair {
        left  = 7,
        right = 1.5,
    }
    pair_pointer := make_pair_pointer(pair)
    array := [2]int{3, 9}
    array_pointer := make_array_pointer(array)
    inner_pointer := make_int_pointer(11)
    pointer_pointer := make_pointer_pointer(inner_pointer)

    int_box := any(int_pointer)
    assert_pointer_box(int_box, rawptr(int_pointer), typeid_of(^int))
    int_value, int_ok := assert_deref(int_box, rawptr(int_pointer), typeid_of(int)).(int)
    assert(int_ok && int_value == 42)

    pair_box := any(pair_pointer)
    assert_pointer_box(pair_box, rawptr(pair_pointer), typeid_of(^Pair))
    pair_value, pair_ok := assert_deref(pair_box, rawptr(pair_pointer), typeid_of(Pair)).(Pair)
    assert(pair_ok && pair_value == pair)

    array_box := any(array_pointer)
    assert_pointer_box(array_box, rawptr(array_pointer), typeid_of(^[2]int))
    array_value, array_ok := assert_deref(array_box, rawptr(array_pointer), typeid_of([2]int)).([2]int)
    assert(array_ok && array_value == array)

    pointer_box := any(pointer_pointer)
    assert_pointer_box(pointer_box, rawptr(pointer_pointer), typeid_of(^^int))
    nested_pointer, nested_ok := assert_deref(pointer_box, rawptr(pointer_pointer), typeid_of(^int)).(^int)
    assert(nested_ok && nested_pointer == inner_pointer)

    calls = 0
    evaluated_box := any(make_int_pointer(99))
    assert(calls == 1)
    evaluated_value, evaluated_ok := reflect.deref(evaluated_box).(int)
    assert(evaluated_ok && evaluated_value == 99)

    nil_pointer: ^int
    nil_box := any(nil_pointer)
    assert_pointer_box(nil_box, nil, typeid_of(^int))
    nil_value := reflect.deref(nil_box)
    assert(nil_value.id == typeid_of(int) && nil_value.data == nil)

    already_boxed := any(17)
    boxed_roundtrip := take_any(already_boxed)
    assert(boxed_roundtrip.id == typeid_of(int) && boxed_roundtrip.data != nil)
    variadic_box := take_variadic(19)
    assert(variadic_box.id == typeid_of(int) && variadic_box.data != nil)
    nil_any := any(nil)
    assert(nil_any.data == nil && nil_any.id == nil)
    exact_int := any(23)
    assert(exact_int.(int) == 23)
    assert(any(pair).(Pair) == pair)
    exact_float := any(2.5)
    assert(exact_float.(f64) == 2.5)
}
