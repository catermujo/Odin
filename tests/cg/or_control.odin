package main

global_error: int

value_or_branch :: proc(value: int) -> (int, bool) {
	return value, value != 2
}

sum_or_break :: proc() -> int {
	total := 0
	for value in 0..<4 {
		current := value_or_branch(value) or_break
		total += current
	}
	return total
}

sum_or_continue :: proc() -> int {
	total := 0
	for value in 0..<4 {
		current := value_or_branch(value) or_continue
		total += current
	}
	return total
}

or_return_proc :: proc(value: int) -> (result: int, ok: bool) {
	current := value_or_branch(value) or_return
	return current + 1, true
}

or_else_value :: proc(value: int) -> int {
	return value_or_branch(value) or_else 99
}

or_else_diverging :: proc(value: int) -> int {
	return value_or_branch(value) or_else panic("unexpected CG or_else fallback")
}

nil_or_branch :: proc(value: int) -> (int, ^int) {
	if value == 2 {
		return 0, &global_error
	}
	return value, nil
}

or_else_nil :: proc(value: int) -> int {
	return nil_or_branch(value) or_else 77
}

map_or_else :: proc(value: int) -> int {
	values := make(map[int]int)
	values[1] = 10
	return (values[value]) or_else 77
}

optional_multi :: proc(ok: bool) -> (a, b: int, found: bool) #optional_ok {
	if ok {
		return 1, 2, true
	}
	return 0, 0, false
}

optional_multi_sum :: proc() -> int {
	a, b := optional_multi(true)
	return a + b
}

multi_or_else_source :: proc(ok: bool) -> (a, b: int, found: bool) {
	return 1, 2, ok
}

multi_or_else_fallback :: proc() -> (int, int) {
	return 10, 20
}

multi_or_else_sum :: proc(ok: bool) -> int {
	a, b := multi_or_else_source(ok) or_else multi_or_else_fallback()
	return a + b
}

main :: proc() {
	if sum_or_break() != 1 {
		panic("CG or_break regression")
	}
	if sum_or_continue() != 4 {
		panic("CG or_continue regression")
	}
	value, ok := or_return_proc(1)
	if value != 2 || !ok {
		panic("CG or_return success regression")
	}
	value, ok = or_return_proc(2)
	if value != 0 || ok {
		panic("CG or_return failure regression")
	}
	if or_else_value(1) != 1 || or_else_value(2) != 99 {
		panic("CG or_else regression")
	}
	if or_else_diverging(1) != 1 {
		panic("CG diverging or_else regression")
	}
	if or_else_nil(1) != 1 || or_else_nil(2) != 77 {
		panic("CG nil or_else regression")
	}
	if map_or_else(1) != 10 || map_or_else(2) != 77 {
		panic("CG map or_else regression")
	}
	if optional_multi_sum() != 3 {
		panic("CG multi optional regression")
	}
	if multi_or_else_sum(true) != 3 || multi_or_else_sum(false) != 30 {
		panic("CG multi or_else regression")
	}
}
