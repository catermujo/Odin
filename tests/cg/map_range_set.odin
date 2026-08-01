package main

map_range_set :: proc() -> int {
	values := make(map[string]struct{})
	values["entry"] = {}
	count := 0
	for key, _ in values {
		count += len(key)
	}
	return count
}

main :: proc() {
	_ = map_range_set()
}
