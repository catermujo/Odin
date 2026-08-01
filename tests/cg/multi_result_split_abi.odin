package main

values :: proc(value: string) -> (string, u32) {
	return value, 7
}

forward :: proc(value: string) -> (string, u32) {
	return values(value)
}

main :: proc() {
	forward("CG")
}
