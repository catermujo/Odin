package main

Mode :: enum u8 {
	zero,
	one,
	two,
}

next_mode :: proc(mode: Mode) -> Mode {
	if mode == .zero {
		return .one
	}
	return .two
}

Pair :: struct {
	left, right: u32,
}

copy_pair :: proc(pair: Pair) -> Pair {
	return pair
}

pair_round_trip :: proc(pair: Pair) -> Pair {
	return copy_pair(pair)
}

string_id :: proc(value: string) -> string {
	return value
}

string_round_trip :: proc(value: string) -> string {
	return string_id(value)
}

slice_id :: proc(value: []u32) -> []u32 {
	return value
}

slice_round_trip :: proc(value: []u32) -> []u32 {
	return slice_id(value)
}

dynamic_array_id :: proc(value: [dynamic]u32) -> [dynamic]u32 {
	return value
}

dynamic_array_round_trip :: proc(value: [dynamic]u32) -> [dynamic]u32 {
	return dynamic_array_id(value)
}

split_values :: proc() -> (u32, u32) {
	return 3, 5
}

split_forward :: proc() -> (u32, u32) {
	return split_values()
}

Vec :: #simd[4]u32

simd_round_trip :: proc(value: Vec) -> Vec {
	copy := value
	return copy
}

main :: proc() {
	mode := Mode.zero
	for _ in 0..<2 {
		mode = next_mode(mode)
	}
	pair: Pair
	pair.left = 3
	pair.right = 5
	pair = pair_round_trip(pair)
	message := string_round_trip("cg")
	values := slice_round_trip([]u32{3, 5})
	dynamic_values := make([dynamic]u32)
	append(&dynamic_values, 3)
	append(&dynamic_values, 5)
	dynamic_capacity := cap(dynamic_values)
	dynamic_values = dynamic_array_round_trip(dynamic_values)
	left, right := split_forward()
	vector := simd_round_trip(Vec{3, 5, 7, 11})
	lanes := transmute([4]u32)vector
	assert(mode == .two)
	assert(pair.left == 3 && pair.right == 5)
	assert(message == "cg")
	assert(len(values) == 2 && values[0] == 3 && values[1] == 5)
	assert(len(dynamic_values) == 2 && cap(dynamic_values) == dynamic_capacity && dynamic_values[0] == 3 && dynamic_values[1] == 5)
	assert(left == 3 && right == 5)
	assert(lanes[0] == 3 && lanes[1] == 5 && lanes[2] == 7 && lanes[3] == 11)
}
