package main

pair :: proc(values: []i32) -> (i32, bool) {
	return i32(len(values)), len(values) != 0
}

conditional_multi_result :: proc() -> i32 {
	values: [dynamic; 4]i32
	if result, ok := pair(values[:]); ok {
		return result
	}
	return 0
}

main :: proc() {
	_ = conditional_multi_result()
}
