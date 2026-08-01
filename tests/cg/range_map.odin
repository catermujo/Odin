package main

count_map :: proc(values: map[u32]i64) -> int {
	count: int
	for _ in values {
		count += 1
	}
	return count
}

sum_map :: proc(values: map[u32]i64) -> i64 {
	total: i64
	for key, value in values {
		total += i64(key) + value
	}
	return total
}

Pair :: struct {
	left, right: i32,
}

increment_map_pairs :: proc(values: map[u32]Pair) {
	for _, &value in values {
		value.left += 1
	}
}

sum_map_pairs :: proc(values: map[u32]Pair) -> i64 {
	total: i64
	for key, value in values {
		total += i64(key) + i64(value.left)
	}
	return total
}

Odd :: struct {
	data: [3]u64,
}

sum_map_odd :: proc(values: map[u32]Odd) -> u64 {
	total: u64
	for _, value in values {
		total += value.data[0]
	}
	return total
}

main :: proc() {
	_ = count_map
	_ = sum_map
	_ = increment_map_pairs
	_ = sum_map_pairs
	_ = sum_map_odd
}
