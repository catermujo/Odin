package main

contains_set_member :: proc(values: ^map[string]struct{}, key: string) -> bool {
	values[key] = {}
	return key in values^
}

main :: proc() {
	values: map[string]struct{}
	_ = contains_set_member(&values, "member")
}
