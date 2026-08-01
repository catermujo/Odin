package main

import "base:intrinsics"

@(export)
sum_varargs :: proc "c" (fixed: int, #c_vararg args: ..any) -> int {
	list: intrinsics.c_va_list
	copy: intrinsics.c_va_list
	intrinsics.c_va_start(&list, args)
	intrinsics.c_va_copy(&copy, &list)
	first := intrinsics.c_va_arg(&list, int)
	second := intrinsics.c_va_arg(&copy, int)
	intrinsics.c_va_end(&copy)
	intrinsics.c_va_end(&list)
	return fixed + first + second
}

result: int

main :: proc() {
	result = sum_varargs(40, 1)
}
