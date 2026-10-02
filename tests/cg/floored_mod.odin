package cg

floored_mod :: proc(value, divisor: i64) -> i64 {
	return value %% divisor
}

floored_mod_assign :: proc(value, divisor: i64) -> i64 {
	result := value
	result %%= divisor
	return result
}

bool_or_assign :: proc(value, other: bool) -> bool {
	result := value
	result |= other
	return result
}
