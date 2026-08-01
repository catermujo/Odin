package main

increment :: proc "contextless" (value: i32) -> i32 {
	return value + 1
}

apply :: proc "contextless" (value: i32, callback: proc "contextless" (i32) -> i32) -> i32 {
	return callback(value)
}

main :: proc() {
	apply(41, increment)
}
