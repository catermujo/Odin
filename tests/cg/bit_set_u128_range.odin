package main

Bits128 :: bit_set[0..<128; u128]
UpperBits128 :: bit_set[64..<128; u128]
SparseFlag :: enum {Low = 0, High = 100}
SparseBits128 :: bit_set[SparseFlag; u128]

@(export)
bit_set_u128_sum :: proc(value: Bits128) -> i64 {
	total: i64
	for index in value {
		total += i64(index)
	}
	return total
}

@(export)
bit_set_u128_upper_sum :: proc(value: UpperBits128) -> i64 {
	total: i64
	for index in value {
		total += i64(index)
	}
	return total
}

@(export)
bit_set_u128_sparse_sum :: proc(value: SparseBits128) -> i64 {
	total: i64
	for index in value {
		total += i64(index)
	}
	return total
}

main :: proc() {}
