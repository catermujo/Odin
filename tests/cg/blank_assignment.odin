package main

value :: proc() -> i32 {
	return 42
}

main :: proc() {
	_ = value()
}
