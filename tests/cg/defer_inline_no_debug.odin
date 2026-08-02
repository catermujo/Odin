package main

defer_order: [8]int
defer_count: int

record_defer :: proc(value: int) {
	defer_order[defer_count] = value
	defer_count += 1
}

reset_defer_order :: proc() {
	defer_order = {}
	defer_count = 0
}

normal_scope_exit :: proc() {
	{
		defer record_defer(1)
		defer record_defer(2)
	}
}

nested_return_exit :: proc() -> int {
	defer record_defer(3)
	{
		defer record_defer(4)
		{
			defer record_defer(5)
			return 42
		}
	}
}

branch_exit :: proc() {
	for {
		defer record_defer(6)
		defer record_defer(7)
		break
	}
}

main :: proc() {
	reset_defer_order()
	normal_scope_exit()
	assert(defer_count == 2)
	assert(defer_order[0] == 2)
	assert(defer_order[1] == 1)

	reset_defer_order()
	assert(nested_return_exit() == 42)
	assert(defer_count == 3)
	assert(defer_order[0] == 5)
	assert(defer_order[1] == 4)
	assert(defer_order[2] == 3)

	reset_defer_order()
	branch_exit()
	assert(defer_count == 2)
	assert(defer_order[0] == 7)
	assert(defer_order[1] == 6)
}
