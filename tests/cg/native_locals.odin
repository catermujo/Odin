package native_locals

@(export)
native_local_initialize :: proc(value: i32) -> i32 {
	local: i32
	local = value + 1
	return local
}

@(export)
native_local_overwrite :: proc(value: i64) -> i64 {
	local: i64
	local = value + 1
	local = local * 3
	return local
}

@(export)
native_local_nested :: proc(a, b: i32) -> i32 {
	lhs: i32
	rhs: i32
	lhs = a + b
	rhs = lhs * 3
	lhs = rhs - 2
	return lhs + rhs
}

main :: proc() {}
