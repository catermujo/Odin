package main

import "core:simd"

V1024 :: #simd[32]u32
V2048 :: #simd[64]u32
V4096 :: #simd[64]u64

sum_1024 :: #force_no_inline proc(value: V1024) -> u32 {
    return simd.extract(value, 0) + simd.extract(value, 15) + simd.extract(value, 31)
}

sum_4096 :: #force_no_inline proc(value: V4096) -> u64 {
    return simd.extract(value, 0) + simd.extract(value, 31) + simd.extract(value, 63)
}

make_1024 :: #force_no_inline proc() -> V1024 {
    value: V1024 = {}
    value = simd.replace(value, 0, 1)
    value = simd.replace(value, 15, 2)
    return simd.replace(value, 31, 3)
}

sum_2048 :: #force_no_inline proc(value: V2048) -> u32 {
    return simd.extract(value, 0) + simd.extract(value, 31) + simd.extract(value, 63)
}

make_2048 :: #force_no_inline proc() -> V2048 {
    value: V2048 = {}
    value = simd.replace(value, 0, 1)
    value = simd.replace(value, 31, 2)
    return simd.replace(value, 63, 3)
}

make_4096 :: #force_no_inline proc() -> V4096 {
    value: V4096 = {}
    value = simd.replace(value, 0, 1)
    value = simd.replace(value, 31, 2)
    return simd.replace(value, 63, 3)
}

main :: proc() {
    assert(sum_1024(make_1024()) == 6)
    assert(sum_2048(make_2048()) == 6)
    assert(sum_4096(make_4096()) == 6)
}
