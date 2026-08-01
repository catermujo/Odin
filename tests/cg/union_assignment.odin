package main

Maybe_I32 :: union {i32}

State :: struct {
	value: Maybe_I32,
}

assign_union :: proc() -> Maybe_I32 {
	value: Maybe_I32
	value = i32(1)
	value = nil
	value = i32(7)
	return value
}

assign_union_field :: proc() -> State {
	state: State
	state.value = i32(2)
	state.value = nil
	state.value = i32(9)
	return state
}

assign_union_value :: proc(value: i32) -> Maybe_I32 {
	result: Maybe_I32
	result = value
	return result
}

assign_union_field_value :: proc(value: i32) -> State {
	state: State
	state.value = value
	return state
}

main :: proc() {
	_ = assign_union()
	_ = assign_union_field()
	_ = assign_union_value(3)
	_ = assign_union_field_value(4)
}
