package main

Base :: [2]i16
Key :: distinct Base
Scalar :: distinct i32

Entry :: struct {
    value: int,
}

key_from_position :: #force_no_inline proc(position: [2]i16, divisor: i16) -> Key {
    base := Base{position[0] / divisor, position[1] / divisor}
    return Key(base)
}

scalar_from_value :: #force_no_inline proc(value: i32) -> Scalar {
    return Scalar(value)
}

packed_key :: #force_no_inline proc(key: Key) -> u32 {
    return transmute(u32)key
}

ordinary_map_capacity :: #force_no_inline proc(requested: int) -> int {
    values := make(map[Key]^Entry, requested)
    result := cap(values)
    delete(values)
    return result
}

generic_map_capacity :: #force_no_inline proc($K: typeid, $V: typeid, requested: int) -> int {
    values := make(map[K]V, requested)
    result := cap(values)
    delete(values)
    return result
}

main :: proc() {
    key := key_from_position([2]i16{4, 6}, 2)
    assert(key[0] == 2)
    assert(key[1] == 3)
    assert(packed_key(key) == 0x00030002)

    scalar := scalar_from_value(42)
    assert(i32(scalar) == 42)

    empty: map[Key]^Entry
    assert(cap(empty) == 0)
    assert(ordinary_map_capacity(4) == 8)
    assert(ordinary_map_capacity(17) == 32)
    assert(generic_map_capacity(int, string, 4) == 8)
    assert(generic_map_capacity(int, string, 17) == 32)

    other := key_from_position([2]i16{4, 8}, 2)
    values := make(map[Key]^Entry, 4)
    assert(len(values) == 0)
    assert(values[key] == nil)

    entry := new(Entry)
    entry.value = 7
    values[key] = entry
    assert(len(values) == 1)

    found, ok := values[key]
    assert(ok)
    assert(found == entry)
    assert(found.value == 7)
    assert(values[other] == nil)

    delete(values)
    free(entry)
}
