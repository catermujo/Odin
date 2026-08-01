package main

import "core:reflect"

First :: struct { value: i32 }
Second :: struct { value: i32 }
Value :: union {First, Second}

main :: proc() {
	value: Value
	reflect.set_union_variant_typeid(value, typeid_of(First))
	value_ptr := &value.(First)
	value_ptr.value = 42
	assert(value.(First).value == 42)
}
