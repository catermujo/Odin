package native_calls

foreign {
	native_external_add :: proc "c" (lhs, rhs: i32) -> i32 ---
	native_external_void :: proc "c" () ---
}

native_internal_add :: proc "c" (lhs, rhs: i32) -> i32 {
	return lhs + rhs
}

@(export)
native_call_internal :: proc "c" (lhs, rhs: i32) -> i32 {
	return native_internal_add(lhs + 1, rhs * 3)
}

@(export)
native_call_forward :: proc "c" (value: i64) -> i64 {
	return native_forward(value, 7)
}

native_forward :: proc "c" (value, addend: i64) -> i64 {
	return value + addend
}

native_eight_args :: proc "c" (a, b, c, d, e, f, g, h: i64) -> i64 {
	return a + h
}

@(export)
native_call_eight_args :: proc "c" (a, b, c, d, e, f, g, h: i64) -> i64 {
	return native_eight_args(a, b, c, d, e, f, g, h)
}

@(export)
native_call_external :: proc "c" (lhs, rhs: i32) -> i32 {
	return native_external_add(lhs, rhs)
}

@(export)
native_call_external_void :: proc "c" () {
	native_external_void()
}

main :: proc() {}
