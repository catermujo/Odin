#+feature using-stmt

package main

import "core:compress"

pair :: proc() -> (int, int) {
	return 20, 22
}

Error_Kind :: enum u8 {none}
Error :: union {Error_Kind}

pair_error :: proc() -> (i64, Error) {
	return 42, .none
}

input_size_value_decl :: proc(using ctx: ^compress.Context_Memory_Input) -> int #no_bounds_check {
	size, size_err := compress.input_size(ctx)
	if size_err != nil {
		return 0
	}
	return int(size)
}

multi_result_value_decl :: proc() -> int #no_bounds_check {
	if true {
		left, right := pair()
		size, size_err := pair_error()
		if size_err != nil {
			return 0
		}
		return left + right + int(size)
	}
	return 0
}

main :: proc() {
	_ = multi_result_value_decl()
	_ = input_size_value_decl(nil)
}
