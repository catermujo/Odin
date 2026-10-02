package main

increment_all :: proc(values: []u8) {
	for &value in values {
		value += 1
	}
}

main :: proc() {
	values := [3]u8{1, 2, 3}
	increment_all(values[:])
}
