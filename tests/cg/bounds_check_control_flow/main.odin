package main

import "core:testing"

RUN_EQUAL_COUNT :: #config(BOUNDS_RUN_EQUAL_COUNT, false)
RUN_NEGATIVE_INDEX :: #config(BOUNDS_RUN_NEGATIVE_INDEX, false)

sum_indexed_values :: #force_no_inline proc(values: []u64) -> u64 {
    output := make([]u64, len(values))
    defer delete(output)
    sum: u64
    for i in 0 ..< len(values) {
        output[i] = values[i] + 1
        sum += output[i]
    }
    return sum
}

@(test)
test_bounds_check_control_flow :: proc(t: ^testing.T) {
    when RUN_EQUAL_COUNT {
        trap_equal_count()
    }
    when RUN_NEGATIVE_INDEX {
        trap_negative_index()
    }

    values := make([]u64, 8)
    defer delete(values)
    for i in 0 ..< len(values) {
        values[i] = u64(i)
    }
    testing.expect_value(t, sum_indexed_values(values), u64(36))
}

trap_equal_count :: proc() {
    values := make([]u64, 8)
    defer delete(values)
    _ = values[len(values)]
}

trap_negative_index :: proc() {
    values := make([]u64, 8)
    defer delete(values)
    index := len(values) - len(values) - 1
    _ = values[index]
}

main :: proc() {
    values := make([]u64, 8)
    defer delete(values)
    _ = sum_indexed_values(values)
}
