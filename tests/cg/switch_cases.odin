package main

classify :: proc(value: i32) -> i32 {
	switch value {
	case 0, 1, 2:
		return 10
	case 3, 4:
		return 20
	case:
		return 30
	}
}

choose :: proc(value: i32) -> i32 {
	switch {
	case value < 0, value == 0:
		return 1
	case value < 10:
		return 2
	case:
		return 3
	}
}

main :: proc() {
	_ = classify(4)
	_ = choose(1)
}
