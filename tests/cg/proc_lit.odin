package main

apply :: proc(value: i32, f: proc(i32) -> i32) -> i32 {
	return f(value)
}

apply_generic :: proc($T: typeid, value: T, f: proc(T) -> T) -> T {
	return f(value)
}

@(export)
twice :: proc(value: i32) -> i32 {
	return apply(value, proc(x: i32) -> i32 {
		return x * 2
	})
}

forward :: proc($T: typeid, value: T) -> T {
	return apply_generic(T, value, proc(x: T) -> T {
		return x
	})
}

main :: proc() {
	_ = twice(21)
	_ = forward(i32, 1)
	_ = forward(u32, 2)
}
