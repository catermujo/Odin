package main

values: [2]u32 = {3, 5}

string_id :: proc(value: string) -> string {
	return value
}

slice_id :: proc(value: []u32) -> []u32 {
	return value
}

main :: proc() {
	string_id("cg")
	slice_id(values[:])
}
