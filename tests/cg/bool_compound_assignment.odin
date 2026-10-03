package main

touch :: proc(counter: ^int) -> bool {
	counter^ += 1
	return true
}

bool_compound_assignment :: proc() -> int {
	value := false
	count := 0
	value &&= touch(&count)
	value ||= touch(&count)
	return count
}

main :: proc() {
	_ = bool_compound_assignment()
}
