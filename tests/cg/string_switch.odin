package main

classify :: proc(value: string) -> i32 {
	switch value {
	case "one", "uno":
		return 1
	case "two":
		return 2
	case:
		return -1
	}
}

main :: proc() {
	_ = classify("two")
}
