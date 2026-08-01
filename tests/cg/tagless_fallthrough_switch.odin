package main

classify :: proc(value: int) -> int {
	switch current := value; {
	case current < 0:
		return 1
	case current > 10:
		current = 10
		fallthrough
	case current == 10:
		return current + 2
	case:
		return 3
	}
}

main :: proc() {
	_ = classify(11)
}
