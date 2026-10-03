package main

import "core:math"

Layout :: enum u8 {
    None,
    Peristyle,
    Other,
}

Building :: struct {
    depth, width: f32,
    layout:       Layout,
}

Large_Building :: struct {
    _:            [12]u8,
    width, depth: f32,
    _:            [44]u8,
    layout:       Layout,
    _:            [40]u8,
}

ceil_to_int :: #force_no_inline proc(value: f32) -> int {
    return max(int(math.ceil(value / 3)), 4)
}

local_counts :: #force_no_inline proc(width, depth: f32) -> int {
    depth_count := max(int(math.ceil(depth / 3)), 4)
    width_count := max(int(math.ceil(width / 3)), 3)
    return depth_count + width_count
}

switch_counts :: #force_no_inline proc(building: Building) -> int {
    switch building.layout {
    case .None:
        return 0
    case .Peristyle:
        depth_count := max(int(math.ceil(building.depth / 3)), 4)
        width_count := max(int(math.ceil(building.width / 3)), 3)
        return depth_count + width_count
    case .Other:
        return 1
    }
    return -1
}

switch_large_counts :: #force_no_inline proc(building: Large_Building) -> int {
    switch building.layout {
    case .None:
        return 0
    case .Peristyle:
        depth_count := max(int(math.ceil(building.depth / 3)), 4)
        width_count := max(int(math.ceil(building.width / 3)), 3)
        return depth_count + width_count
    case .Other:
        return 1
    }
    return -1
}

switch_large_loop_counts :: #force_no_inline proc(building: Large_Building) -> int {
    switch building.layout {
    case .None:
        return 0
    case .Peristyle:
        depth_count := max(int(math.ceil(building.depth / 3)), 4)
        width_count := max(int(math.ceil(building.width / 3)), 3)
        index := 0
        for side in ([2]f32{-1, 1}) {
            for column in 0 ..< depth_count {
                z := -building.depth * .40 + building.depth * .80 * f32(column) / f32(depth_count - 1)
                index += int(side + z) + column
            }
        }
        return index + width_count
    case .Other:
        return 1
    }
    return -1
}

main :: proc() {
    assert(ceil_to_int(12) == 4)
    assert(ceil_to_int(15) == 5)
    assert(local_counts(15, 12) == 9)
    assert(switch_counts({depth = 12, width = 15, layout = .Peristyle}) == 9)
    assert(switch_large_counts({width = 15, depth = 12, layout = .Peristyle}) == 9)
    assert(switch_large_loop_counts({width = 15, depth = 12, layout = .Peristyle}) == 17)
}
