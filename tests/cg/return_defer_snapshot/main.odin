package main

import "core:testing"

Pair :: struct {
    left:  i32,
    right: i32,
}

plain_scalar :: proc() -> int {
    return 11
}

plain_split :: proc() -> string {
    return "plain"
}

plain_multi :: proc() -> (left, right: int) {
    return 21, 22
}

plain_aggregate :: proc() -> Pair {
    return Pair{31, 32}
}

scalar_local :: proc() -> int {
    value := 10
    defer value += 1
    return value
}

named_explicit :: proc() -> (value: int) {
    value = 20
    defer value += 1
    return value + 5
}

two_results :: proc() -> (left, right: int) {
    first := 30
    second := 40
    defer first += 1
    return first, second
}

forwarded_source :: proc() -> (left, right: int) {
    return 35, 45
}

forwarded_results :: proc() -> (left, right: int) {
    defer left += 1
    return forwarded_source()
}

string_result :: proc() -> string {
    value := "before"
    defer value = "after"
    return value
}

aggregate_result :: proc() -> Pair {
    value := Pair {
        left  = 50,
        right = 60,
    }
    defer value.left += 1
    return value
}

array_result :: proc() -> [2]i32 {
    value := [2]i32{70, 80}
    defer value[0] += 1
    return value
}

loop_body :: proc() -> int {
    count := 0
    for i in 0 ..< 10 {
        _ = i
        defer count += 1
        return count
    }
    return count
}

fallible_source :: proc(ok: bool) -> (value: int, result_ok: bool) {
    return 7, ok
}

or_return_snapshot :: proc(ok: bool) -> (value: int, result_ok: bool) {
    defer value += 1
    current := fallible_source(ok) or_return
    return current, true
}

@(test)
test_return_defer_snapshot :: proc(t: ^testing.T) {
    _ = t
    assert(plain_scalar() == 11)
    assert(plain_split() == "plain")
    plain_left, plain_right := plain_multi()
    assert(plain_left == 21 && plain_right == 22)
    assert(plain_aggregate() == Pair{31, 32})
    assert(scalar_local() == 10)
    assert(named_explicit() == 25)
    two_left, two_right := two_results()
    assert(two_left == 30 && two_right == 40)
    forwarded_left, forwarded_right := forwarded_results()
    assert(forwarded_left == 35 && forwarded_right == 45)
    assert(string_result() == "before")
    assert(aggregate_result() == Pair{50, 60})
    assert(array_result() == [2]i32{70, 80})
    assert(loop_body() == 0)
    success_value, success_ok := or_return_snapshot(true)
    assert(success_value == 7 && success_ok)
    failure_value, failure_ok := or_return_snapshot(false)
    assert(failure_value == 0 && !failure_ok)
}

main :: proc() {  }
