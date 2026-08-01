package main

import "core:testing"

reachable_helper :: proc "contextless" (value: i32) -> i32 {
    return value + 1
}

unused_package_local_helper :: #force_no_inline proc "contextless" () -> i32 {
    return 7
}

unused_package_local_use :: proc "contextless" () {
    _ = unused_package_local_helper()
}

@(test)
test_reachable_helper :: proc(t: ^testing.T) {
    testing.expect_value(t, reachable_helper(41), i32(42))
}

main :: proc() {  }
