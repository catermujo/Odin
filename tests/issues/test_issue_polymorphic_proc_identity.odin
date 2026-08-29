package main
import "core:fmt"
first :: proc(alpha: int) {  }
second :: proc(beta: int) {  }
get_type :: proc(f: $T) -> typeid { return T }
main :: proc() {
    a := get_type(first)
    b := get_type(second)
    fmt.println(a, b, typeid_of(type_of(first)), typeid_of(type_of(second)))
    assert(a == typeid_of(type_of(first)))
    assert(b == typeid_of(type_of(second)))
}
