package main

main :: proc() {}

@(export)
closure_by_value :: proc(value: i64) -> i64 {
	bias := i64(7)
	add := lambda [bias](x: i64) -> i64 { return bias + x }
	return add(value)
}

@(export)
closure_by_ref :: proc(value: i64) -> i64 {
	counter := value
	bump := lambda [&counter]() { counter += 1 }
	bump()
	bump()
	return counter
}

apply :: proc(f: lambda(i64) -> i64, value: i64) -> i64 {
	return f(value)
}

@(export)
closure_as_argument :: proc(value: i64) -> i64 {
	bias := i64(5)
	add := lambda [bias](x: i64) -> i64 { return bias + x }
	return apply(add, value)
}

make_counter :: proc(value: i64) -> lambda() -> i64 {
	counter := value
	bump := lambda [counter]() -> i64 { counter += 1; return counter }
	return closure_clone(bump, context.allocator)
}

@(export)
closure_clone_free :: proc(value: i64) -> i64 {
	counter := make_counter(value)
	result := counter()
	closure_free(counter, context.allocator)
	return result
}

@(export)
closure_clone_free_local :: proc(value: i64) -> i64 {
	counter := value
	bump := lambda [counter]() -> i64 { counter += 1; return counter }
	cloned := closure_clone(bump, context.allocator)
	result := cloned()
	closure_free(cloned, context.allocator)
	return result
}
