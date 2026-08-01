package cg

next :: proc(state: ^int) -> (int, int, bool) {
	if state^ >= 3 {
		return 0, 0, false
	}
	value := state^
	state^ += 1
	return value, value*2, true
}

Wrap :: struct {
	data: [3]int,
}

@(operator="in")
wrap_next :: proc(w: Wrap, state: ^int) -> (int, bool) {
	if state^ >= len(w.data) {
		return 0, false
	}
	value := w.data[state^]
	state^ += 1
	return value, true
}

iterate :: proc() -> int {
	sum := 0
	for skipped := 0; value, doubled in next(&skipped) {
		if value == 1 {
			continue
		}
		sum += doubled
	}
	return sum
}

iterate_overload :: proc() -> int {
	w := Wrap{data = {2, 3, 4}}
	sum := 0
	for offset := 10; value in w {
		sum += value + offset
		offset += 1
	}
	return sum
}

pointer_length :: proc(s: ^string) -> int {
	return len(s)
}

main :: proc() {
	_ = iterate()
	_ = iterate_overload()
	text := "abc"
	_ = pointer_length(&text)
}
