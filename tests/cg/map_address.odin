package main

find_map_value :: proc(values: ^map[int]int, key: int) -> ^int {
	find := &values[key]
	if find == nil {
		return nil
	}
	return find
}

main :: proc() {
	values: map[int]int
	_ = find_map_value(&values, 7)
}
