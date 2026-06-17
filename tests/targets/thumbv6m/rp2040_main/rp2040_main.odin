package main

initialized_value : i32 = 0x1234
zero_value        : i32

@(export)
main :: proc "c"() -> i32 {
	zero_value = initialized_value
	return zero_value
}
