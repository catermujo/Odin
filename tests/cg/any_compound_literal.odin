package main

@(export)
make_any :: proc(value: ^i32, id: typeid) -> any {
	return any{rawptr(value), id}
}

main :: proc() {
}
