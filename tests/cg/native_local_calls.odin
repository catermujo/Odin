package native_local_calls

foreign {
	native_local_external_add :: proc "c" (lhs, rhs: i32) -> i32 ---
	native_local_external_void :: proc "c" () ---
}

native_local_internal_add :: proc "c" (lhs, rhs: i32) -> i32 {
	return lhs + rhs
}

@(export)
native_local_call_internal :: proc "c" (value: i32) -> i32 {
	local: i32
	local = value + 1
	return native_local_internal_add(local + 2, 7)
}

@(export)
native_local_call_foreign :: proc "c" (value: i32) -> i32 {
	local: i32
	local = value * 2
	return native_local_external_add(local, 3)
}

@(export)
native_local_call_void :: proc "c" (value: i32) {
	local: i32
	local = value + 5
	native_local_external_void()
}

main :: proc() {}
