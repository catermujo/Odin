package closure_helper

Callback_Context :: struct {
	callback: proc(value: int) -> int,
}

direct :: proc(value: int) -> int {
	return value + 1
}

callback_only :: proc(value: int) -> int {
	return package_local_helper(value)
}

global_callback: proc(value: int) -> int = global_callback_target

global_callback_target :: proc(value: int) -> int {
	return value + 3
}

package_local_helper :: proc(value: int) -> int {
	return value + 2
}

callback_generated :: proc(value: int) -> int {
	return generated_helper(int, value)
}

generated_helper :: proc($T: typeid, value: T) -> T {
	return value + T(2)
}

unused_package_local_helper :: proc(value: int) -> int {
	return value - 100
}
