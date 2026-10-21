package main

check_context :: proc(expected: int) {
	assert(context.user_index == expected)
}

probe :: proc(early: bool) {
	saved := context.user_index
	defer context.user_index = saved
	context.user_index = 123
	defer check_context(123)
	if early do return
	check_context(123)
}

main :: proc() {
	context.user_index = 456
	probe(false)
	check_context(456)
	probe(true)
	check_context(456)
}
