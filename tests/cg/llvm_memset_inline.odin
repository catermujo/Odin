package cg

@(export)
small_zero_init :: proc "c" () -> u64 {
	value: [4]u64
	value[0] = 73
	return value[1] + value[2] + value[3]
}

main :: proc() {
	assert(small_zero_init() == 0)
}
