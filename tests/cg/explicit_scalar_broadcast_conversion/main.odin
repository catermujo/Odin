package main

import "core:simd"

Pos :: distinct [2]i16

u8_calls: int
i16_calls: int
f32_calls: int

named_u8: u8 : 0x5a
named_i16: i16 : -7
named_f32: f32 : 0.5

make_u8 :: #force_no_inline proc(value: u8) -> u8 {
    u8_calls += 1
    return value
}

make_i16 :: #force_no_inline proc(value: i16) -> i16 {
    i16_calls += 1
    return value
}

make_f32 :: #force_no_inline proc(value: f32) -> f32 {
    f32_calls += 1
    return value
}

splat_u8 :: proc(value: u8) -> simd.u8x16 {
    return simd.u8x16(value)
}

splat_i16 :: proc(value: i16) -> simd.i16x8 {
    return simd.i16x8(value)
}

splat_f32 :: proc(value: f32) -> simd.f32x4 {
    return simd.f32x4(value)
}

splat_pos :: proc(value: i16) -> Pos {
    return Pos(value)
}

assert_pos_splat :: proc(value: Pos, expected: i16) {
    assert(value[0] == expected && value[1] == expected)
}

assert_u8_splat :: proc(value: simd.u8x16, expected: u8) {
    assert(simd.reduce_and(simd.lanes_eq(value, simd.u8x16(expected))) != 0)
}

assert_i16_splat :: proc(value: simd.i16x8, expected: i16) {
    assert(simd.reduce_and(simd.lanes_eq(value, simd.i16x8(expected))) != 0)
}

assert_f32_splat :: proc(value: simd.f32x4, expected: f32) {
    assert(simd.reduce_and(simd.lanes_eq(value, simd.f32x4(expected))) != 0)
}

first_match_after :: proc(values: #simd[16]u8, needle: u8, after: int) -> int {
    indices: #simd[16]u8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}
    equals := simd.lanes_eq(values, simd.u8x16(needle))
    positive := simd.lanes_gt(values, simd.u8x16(0))
    mask := simd.bit_and(equals, positive)
    if after >= 0 {
        mask = simd.bit_and(mask, simd.lanes_gt(indices, simd.u8x16(u8(after))))
    }
    selected := simd.select(mask, indices, simd.u8x16(0xff))
    result := int(simd.reduce_min(selected))
    return result < 16 ? result : -1
}

main :: proc() {
    u8_calls = 0
    i16_calls = 0
    f32_calls = 0

    assert_u8_splat(splat_u8(make_u8(0x5a)), 0x5a)
    assert_i16_splat(splat_i16(make_i16(-7)), -7)
    assert_f32_splat(splat_f32(make_f32(0.5)), 0.5)
    assert_pos_splat(splat_pos(make_i16(6)), 6)
    assert(u8_calls == 1)
    assert(i16_calls == 2)
    assert(f32_calls == 1)

    assert_pos_splat(Pos(9), 9)
    assert_u8_splat(simd.u8x16(7), 7)
    assert_u8_splat(simd.u8x16(named_u8), named_u8)
    assert_i16_splat(simd.i16x8(-3), -3)
    assert_i16_splat(simd.i16x8(named_i16), named_i16)
    assert_f32_splat(simd.f32x4(0.25), 0.25)
    assert_f32_splat(simd.f32x4(named_f32), named_f32)

    values := #simd[16]u8{1, 2, 3, 0x5a, 5, 6, 7, 8, 9, 10, 11, 12, 0x5a, 14, 15, 16}
    assert(first_match_after(values, 0x5a, -1) == 3)
    assert(first_match_after(values, 0x5a, 3) == 12)
    assert(first_match_after(values, 0x5a, 12) == -1)
    assert(first_match_after(values, 0xee, -1) == -1)
    assert(first_match_after(values, 0, -1) == -1)
}
