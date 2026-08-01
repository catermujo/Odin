package native_comparisons

@(export)
native_i32_eq :: proc(lhs, rhs: i32) -> i32 {
	if lhs == rhs {
		return 1
	}
	return 0
}

@(export)
native_i32_ne :: proc(lhs, rhs: i32) -> i32 {
	if lhs != rhs {
		return 0
	}
	return 1
}

@(export)
native_u32_ult :: proc(lhs, rhs: u32) -> u32 {
	if lhs < rhs {
		return 1
	}
	return 0
}

@(export)
native_u32_ule :: proc(lhs, rhs: u32) -> u32 {
	if lhs <= rhs {
		return lhs + 1
	}
	return rhs - 1
}

@(export)
native_i64_ilt :: proc(lhs, rhs: i64) -> i64 {
	if lhs < rhs {
		return 1
	}
	return 0
}

@(export)
native_i64_ile :: proc(lhs, rhs: i64) -> i64 {
	if lhs <= rhs {
		return lhs + 7
	}
	return rhs - 3
}

@(export)
native_u64_ult :: proc(lhs, rhs: u64) -> u64 {
	if lhs < rhs {
		return lhs ~ rhs
	}
	return lhs | rhs
}

@(export)
native_i64_nested_compare :: proc(lhs, rhs: i64) -> i64 {
	if lhs + 7 <= rhs - 3 {
		return (lhs + 1) * (rhs - 2)
	}
	return lhs ~ rhs
}

main :: proc() {}
