package main

add :: proc(x, y: u32) -> u32 {
	return x + y
}

main :: proc() {
	value := add(20, 22)
	if value != 42 {
		return
	}
}
