package main

classify :: proc(value: int) -> int {
	result := 0
	switch value {
	case 0:
		result += 1
		fallthrough
	case 1:
		result += 2
	case:
		result = 7
	}
	return result
}

main :: proc() {
	_ = classify(0)
}
