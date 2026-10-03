package main

soa_range_loop :: proc(names: []string, values: []int) -> int {
	fields := soa_zip(name=names, value=values)
	total := 0
	for field in fields {
		total += len(field.name) + field.value
	}
	return total
}

main :: proc() {
	names := [2]string{"a", "bb"}
	values := [2]int{1, 2}
	_ = soa_range_loop(names[:], values[:])
}
