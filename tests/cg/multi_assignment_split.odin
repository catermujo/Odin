package main

split_assignment :: proc(input: string) -> int {
	first: rune
	rest: string
	first, rest = rune(input[0]), input[1:]
	return int(first) + len(rest)
}

subslice_assignment :: proc(input: []int) -> int {
	first, rest := input[0:1], input[1:]
	return len(first) + len(rest)
}

main :: proc() {
	_ = split_assignment("abc")
	values := [3]int{1, 2, 3}
	_ = subslice_assignment(values[:])
}
