package main

@(rodata)
integer_value: any = i64(7)

@(rodata)
string_value: any = "cg"

@(rodata)
array_value: any = [2]i32{3, 5}

@(export)
global_constant_any :: proc "c" () -> i64 {
	return integer_value.(i64) + i64(len(string_value.(string))) + i64(array_value.([2]i32)[1])
}

main :: proc() {}
