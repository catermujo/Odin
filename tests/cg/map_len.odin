package main

map_len_after_insert :: proc() -> int {
	values := make(map[string]int)
	values["entry"] = 7
	return len(values)
}

main :: proc() {
	_ = map_len_after_insert()
}
