package main

Value :: union {int}
Container :: struct {value: Value}

optional_assertion :: #force_inline proc(container: ^Container) -> int {
	if result, ok := &container.value.?; ok {
		return result^
	}
	return 0
}

main :: proc() {
	container := Container{Value(1)}
	_ = optional_assertion(&container)
}
