package main

Large :: [1025]u32

large_result: Large
attempts: u32
observed: u32

make_large :: proc() -> (Large, u32) {
	defer {
		observed = large_result[0]
	}
	result: Large
	result[0] = 7
	result[1024] = 99
	return result, 42
}

make_optional_large :: proc() -> (Large, u32, bool) #optional_ok {
	result: Large
	result[0] = 11
	result[1024] = 101
	return result, 84, true
}

main :: proc() {
	large_result[0] = 1
	large_result, attempts = make_large()
	assert(large_result[0] == 7)
	assert(large_result[1024] == 99)
	assert(attempts == 42)
	assert(observed == 1)

	large_result, attempts = make_optional_large()
	assert(large_result[0] == 11)
	assert(large_result[1024] == 101)
	assert(attempts == 84)

	_, attempts = make_large()
	assert(attempts == 42)
}
