package main

initialize_later :: proc() -> u32 {
	value: u32 = ---
	value = 7
	return value
}

main :: proc() {
	_ = initialize_later()
}
