package main

soa_zip_range :: proc(names: []string, values: []int) -> int {
	fields := soa_zip(name=names, value=values)
	unpacked_names, unpacked_values := soa_unzip(fields)
	total := 0
	for field in fields {
		total += len(field.name) + field.value
	}
	return total + len(unpacked_names) + unpacked_values[0]
}

main :: proc() {
	names := [2]string{"a", "bb"}
	values := [2]int{1, 2}
	_ = soa_zip_range(names[:], values[:])
}
