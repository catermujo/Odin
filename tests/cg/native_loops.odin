package native_loops

foreign {
	native_loop_external_void :: proc "c" () ---
	native_loop_sink :: proc "c" (value: i32) ---
}

@(export)
native_loop_countdown :: proc "c" (value: i32) -> i32 {
	total := value
	i: i32 = 5
	for i != 0 {
		total += i
		i -= 1
	}
	return total
}

@(export)
native_loop_accumulator :: proc "c" (value: i64) -> i64 {
	total := value
	i: i64 = 1
	for i <= 8 {
		total += i
		i += 1
	}
	return total
}

@(export)
native_loop_nested_if :: proc "c" (value: i32) -> i32 {
	total := value
	for i: i32 = 0; i < 10; i += 1 {
		if i == 3 {
			continue
		}
		if i == 8 {
			break
		}
		total += i
	}
	return total
}

@(export)
native_loop_void_call :: proc "c" (count: i32) -> i32 {
	i: i32 = 0
	for i < count {
		native_loop_external_void()
		i += 1
	}
	return i
}

@(export)
native_loop_sink_order :: proc "c" (count: i32) -> i32 {
	i: i32 = 0
	for i < count {
		native_loop_sink(i)
		i += 1
	}
	return i
}

@(export)
native_scalar_merge_phi :: proc "c" (condition, lhs, rhs: i32) -> i32 {
	return lhs if condition != 0 else rhs
}

@(export)
native_scalar_merge_phi_i64 :: proc "c" (condition: i32, lhs, rhs: i64) -> i64 {
	return lhs if condition != 0 else rhs
}

main :: proc() {}
