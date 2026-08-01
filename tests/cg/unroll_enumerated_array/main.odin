package main

import "core:testing"

Cardinal :: enum u8 {
    left,
    right,
    down,
    up,
}

OFFSETS :: [Cardinal][2]i16 {
    .left  = {-1, 0},
    .right = {+1, 0},
    .down  = {0, +1},
    .up    = {0, -1},
}

unrolled_offset_sum :: proc() -> i16 {
    result: i16
    #unroll for offset, side in OFFSETS {
        if side == .left || side == .right {
            result += offset[0]
        } else {
            result += offset[1]
        }
    }
    return result
}

@(test)
test_unrolled_offset_sum :: proc(t: ^testing.T) {
    testing.expect_value(t, unrolled_offset_sum(), i16(0))
}

main :: proc() {  }
