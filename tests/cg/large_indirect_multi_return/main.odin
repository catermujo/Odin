package main

import "core:testing"

Large :: struct {
	data: [4096]u64,
}

Small :: struct {
	left: u64,
	right: u64,
}

large_result :: proc(select: bool) -> (Large, bool) {
	value: Large
	if select {
		value.data[0] = 11
		value.data[2048] = 23
		value.data[4095] = 47
		return value, true
	}
	value.data[0] = 13
	value.data[2048] = 25
	value.data[4095] = 49
	return value, false
}

large_result_final_indirect :: proc(select: bool) -> (bool, Large) {
	value: Large
	if select {
		value.data[0] = 17
		value.data[2048] = 29
		value.data[4095] = 53
		return true, value
	}
	value.data[0] = 19
	value.data[2048] = 31
	value.data[4095] = 55
	return false, value
}

small_result :: proc(select: bool) -> Small {
	if select {
		return Small{3, 5}
	}
	return Small{7, 11}
}

check_large :: proc(select: bool, expected: u64, expected_flag: bool) {
	value, flag := large_result(select)
	assert(value.data[0] == expected)
	assert(value.data[2048] == expected + 12)
	assert(value.data[4095] == expected + 36)
	assert(flag == expected_flag)
}

check_large_final_indirect :: proc(select: bool, expected: u64, expected_flag: bool) {
	flag, value := large_result_final_indirect(select)
	assert(value.data[0] == expected)
	assert(value.data[2048] == expected + 12)
	assert(value.data[4095] == expected + 36)
	assert(flag == expected_flag)
}

main :: proc() {
	check_large(true, 11, true)
	check_large(false, 13, false)
	check_large_final_indirect(true, 17, true)
	check_large_final_indirect(false, 19, false)
	assert(small_result(true) == Small{3, 5})
	assert(small_result(false) == Small{7, 11})
}

@(test)
test_large_indirect_multi_return :: proc(t: ^testing.T) {
	_ = t
	main()
}
