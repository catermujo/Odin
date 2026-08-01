package main

Item :: struct {
	value: int,
}

empty_pointer :: proc() -> ^Item {
	return {}
}

new_pointer :: proc() -> ^Item {
	return new(Item)
}

pointer_result :: proc(value: ^Item) -> (^Item, bool) {
	return value, true
}

conditional_pointer_result :: proc(fail: bool, value: ^Item) -> (^Item, bool) {
	if fail {
		return {}, false
	}
	return value, true
}

main :: proc() {
	item := Item{value = 1}
	_ = empty_pointer()
	_ = new_pointer()
	_, _ = pointer_result(&item)
	_, _ = conditional_pointer_result(true, &item)
}
