package main

Loaded :: #load("raw_data_load.odin")

loaded_data :: proc() -> rawptr {
	return raw_data(Loaded)
}

loaded_size :: proc() -> int {
	return len(Loaded)
}

main :: proc() {
	_ = loaded_data()
	_ = loaded_size()
}
