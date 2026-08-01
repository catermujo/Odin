package main

count :: proc(values: ..i32, loc := #caller_location) -> int {
	return len(values) + int(loc.line > 0)
}

forward :: proc(values: []i32, loc := #caller_location) -> int {
	return count(..values, loc=loc)
}

forward_named :: proc(values: []i32, loc := #caller_location) -> int {
	return count(values=values, loc=loc)
}

main :: proc() {
	values := []i32{3, 5, 7}
	forward(values)
	forward_named(values)
	count(1, 2)
}
