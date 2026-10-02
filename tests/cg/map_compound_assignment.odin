package main

compound_map :: proc(values: ^map[int]int, key: int) {
	values[key] -= 1
	values[key] += 2
}

main :: proc() {
	values: map[int]int
	compound_map(&values, 7)
}
