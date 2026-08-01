package main

main :: proc() {
	context.user_index = 456
	{
		context.user_index = 123
		check_context()
	}
	assert(context.user_index == 456)

	check_context :: proc() {
		assert(context.user_index == 123)
	}
}
