package main

import "core:testing"

Axis :: enum u8 {
    x,
    y,
    z,
}

Flag :: enum u8 {
    active,
    visible,
}

Flags :: bit_set[Flag;u32]

Element :: struct {
    value:  i64,
    coords: [2]f32,
    axis:   Axis,
    flags:  Flags,
}

Large :: struct {
    elements: [100_000]Element,
}

Nested :: struct {
    rows: [2][100_000]Element,
}

Distinct_Array :: distinct [4]u32

make_element :: proc(index: int) -> Element {
    flags := Flags{}
    if index % 2 == 0 {
        flags += {.active}
    }
    if index % 3 == 0 {
        flags += {.visible}
    }
    return Element{value = i64(index), coords = {f32(index), -f32(index)}, axis = Axis(index % 3), flags = flags}
}

test_small_controls :: proc() {
    plain_left := [4]u32{1, 2, 3, 4}
    plain_right := [4]u32{1, 2, 3, 4}
    assert(plain_left == plain_right)
    plain_right[2] += 1
    assert(plain_left != plain_right)

    distinct_left := Distinct_Array{1, 2, 3, 4}
    distinct_right := Distinct_Array{1, 2, 3, 4}
    assert(distinct_left == distinct_right)
    distinct_right[2] += 1
    assert(distinct_left != distinct_right)

    enumerated_left := [Axis]u32 {
        .x = 1,
        .y = 2,
        .z = 3,
    }
    enumerated_right := [Axis]u32 {
        .x = 1,
        .y = 2,
        .z = 3,
    }
    assert(enumerated_left == enumerated_right)
    enumerated_right[.z] += 1
    assert(enumerated_left != enumerated_right)
}

test_large_equality :: proc() {
    left := new(Large)
    right := new(Large)
    defer free(left)
    defer free(right)

    for index in 0 ..< len(left.elements) {
        left.elements[index] = make_element(index)
        right.elements[index] = left.elements[index]
    }

    assert(left^ == left^)
    assert(left^ == right^)

    right.elements[0].value += 1
    assert(left^ != right^)
    right.elements[0] = left.elements[0]

    middle := len(left.elements) / 2
    right.elements[middle].coords[1] += 1
    assert(left^ != right^)
    right.elements[middle] = left.elements[middle]

    last := len(left.elements) - 1
    right.elements[last].flags -= {.visible}
    assert(left^ != right^)
    right.elements[last] = left.elements[last]

    right.elements[1].coords[0] = -0.0
    left.elements[1].coords[0] = 0.0
    assert(left^ == right^)
    right.elements[1] = left.elements[1]

    left.elements[2].coords[0] = f32(0h7fc0_0000)
    right.elements[2].coords[0] = f32(0h7fc0_0000)
    assert(left^ != right^)
    left.elements[2] = make_element(2)
    right.elements[2] = left.elements[2]

    nested_left := new(Nested)
    nested_right := new(Nested)
    defer free(nested_left)
    defer free(nested_right)
    nested_left.rows[0][0] = make_element(7)
    nested_right.rows[0][0] = nested_left.rows[0][0]
    nested_left.rows[1][last] = make_element(11)
    nested_right.rows[1][last] = nested_left.rows[1][last]
    assert(nested_left^ == nested_right^)
    nested_right.rows[1][last].value += 1
    assert(nested_left^ != nested_right^)
}

@(test)
test_large_aggregate_equality :: proc(t: ^testing.T) {
    _ = t
    test_small_controls()
    test_large_equality()
}

main :: proc() {
    test_small_controls()
    test_large_equality()
}
