package main

identity :: proc($T: typeid, value: T) -> T {
	return value
}

main :: proc() {
	identity(i32, 42)
}
