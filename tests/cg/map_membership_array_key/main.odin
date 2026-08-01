package main

import "core:testing"

array_key_membership :: proc() -> bool {
    m: map[[2]i16]bool
    present := [2]i16{1, 2}
    absent := [2]i16{3, 4}
    m[present] = true
    return present in m && absent not_in m
}

@(test)
test_array_key_membership :: proc(t: ^testing.T) {
    testing.expect_value(t, array_key_membership(), true)
}

main :: proc() {  }
