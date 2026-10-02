package main

sum_fixed_dynamic :: proc(values: [dynamic; 8]i32) -> i64 {
	total: i64
	for value, index in values {
		total += i64(value) + i64(index)
	}
	return total
}

sum_fixed_dynamic_reverse :: proc(values: [dynamic; 8]i32) -> i64 {
	total: i64
	#reverse for value, index in values {
		total += i64(value) + i64(index)
	}
	return total
}

sum_fixed_dynamic_pointer :: proc(values: ^[dynamic; 8]i32) -> i64 {
	total: i64
	for value, index in values {
		total += i64(value) + i64(index)
	}
	return total
}

increment_fixed_dynamic :: proc(values: ^[dynamic; 8]i32) {
	for &value in values {
		value += 1
	}
}

main :: proc() {
	_ = sum_fixed_dynamic
	_ = sum_fixed_dynamic_reverse
	_ = sum_fixed_dynamic_pointer
	_ = increment_fixed_dynamic
}
