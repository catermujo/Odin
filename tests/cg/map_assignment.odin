package main

assign_map :: proc(key, value: i32) {
	values: map[i32]i32
	values[key] = value
}

State :: struct {
	position: [2]i32,
	name:     string,
}

assign_map_aggregate :: proc(key: ^i32, value: State) {
	values: map[^i32]State
	values[key] = value
}

lookup_map :: proc(key: i32) -> i32 {
	values: map[i32]i32
	values[5] = 9
	return values[key]
}

lookup_map_ok :: proc(key: i32) -> bool {
	values: map[i32]i32
	values[5] = 9
	_, ok := values[key]
	return ok
}

lookup_map_pair :: proc(key: i32) -> (i32, bool) {
	values: map[i32]i32
	values[5] = 9
	return values[key]
}

lookup_map_address :: proc(key: i32) -> ^i32 {
	values: map[i32]i32
	values[5] = 9
	value, _ := &values[key]
	return value
}

lookup_map_address_ok :: proc(key: i32) -> bool {
	values: map[i32]i32
	values[5] = 9
	_, ok := &values[key]
	return ok
}

lookup_map_address_pair :: proc(key: i32) -> (^i32, bool) {
	values: map[i32]i32
	values[5] = 9
	return &values[key]
}

lookup_map_address_pointer :: proc(values: ^map[i32]i32, key: i32) -> ^i32 {
	value, _ := &values[key]
	return value
}

lookup_map_pointer :: proc(values: ^map[i32]i32, key: i32) -> i32 {
	return values[key]
}

lookup_map_value :: proc(values: map[i32]i32, key: i32) -> i32 {
	return values[key]
}

map_contains :: proc(values: map[i32]i32, key: i32) -> bool {
	return key in values
}

map_lacks :: proc(values: ^map[i32]i32, key: i32) -> bool {
	return key not_in values
}

assign_map_pointer :: proc(values: ^map[i32]i32, key, value: i32) {
	values[key] = value
}

lookup_map_aggregate :: proc(key: ^i32) -> State {
	values: map[^i32]State
	values[key] = {position = {3, 8}}
	return values[key]
}

capture_any :: proc(values: ..any) -> int {
	return len(values)
}

lookup_map_field :: proc(values: map[i32]State, key: i32) -> string {
	return values[key].name
}

lookup_map_field_any :: proc(values: map[i32]State, key: i32) -> int {
	return capture_any(values[key].name)
}

main :: proc() {
	assign_map(3, 7)
	value := i32(11)
	assign_map_aggregate(&value, {position = {2, 5}})
	_ = lookup_map(5)
	_ = lookup_map_ok(4)
	_, _ = lookup_map_pair(5)
	_ = lookup_map_address(5)
	_ = lookup_map_address_ok(4)
	_, _ = lookup_map_address_pair(5)
	values: map[i32]i32
	assign_map_pointer(&values, 5, 9)
	_ = lookup_map_address_pointer(&values, 5)
	_ = lookup_map_pointer(&values, 5)
	_ = lookup_map_value(values, 5)
	_ = map_contains(values, 5)
	_ = map_lacks(&values, 5)
	_ = lookup_map_aggregate(&value)
	state_values: map[i32]State
	_ = lookup_map_field(state_values, 5)
	_ = lookup_map_field_any(state_values, 5)
}
