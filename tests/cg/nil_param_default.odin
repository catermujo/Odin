package main

Option :: enum u8 {
	None,
	Some,
}

enum_default :: proc(value: Option = nil) -> Option {
	return value
}

slice_default :: proc(value: []u8 = nil) -> int {
	return len(value)
}

main :: proc() {
	_ = enum_default()
	_ = slice_default()
}
