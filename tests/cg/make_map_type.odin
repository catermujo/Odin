package main

Entry :: struct {
	value: int,
}

make_map_type :: proc() {
	values: map[string]Entry
	values = make(type_of(values))
	values["entry"] = {value = 7}
}

main :: proc() {
	make_map_type()
}
