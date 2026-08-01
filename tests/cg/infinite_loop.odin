package main

classify :: proc(value: int) -> int {
	for {
		if value > 0 {
			return value
		}
		return -value
	}
}

main :: proc() {
	_ = classify(-3)
	_ = classify(5)
}
