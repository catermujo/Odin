package cg

Pair :: struct { left, right: i32 }

choose_pointer :: proc(left, right: ^i32, use_left: bool) -> ^i32 {
	return left if use_left else right
}

choose_slice :: proc(left, right: []i32, use_left: bool) -> []i32 {
	return left if use_left else right
}

choose_pair :: proc(left, right: Pair, use_left: bool) -> Pair {
	return left if use_left else right
}

pair :: proc(value: ^i32) -> (i32, bool) {
	return value^, value != nil
}

forward_pair :: proc(left, right: ^i32, use_left: bool) -> (i32, bool) {
	return pair(left if use_left else right)
}
