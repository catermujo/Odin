package main

context_allocator_data :: proc() -> rawptr {
	return context.allocator.data
}

main :: proc() {
	context_allocator_data()
}
