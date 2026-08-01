package main

message: string = "CG relocation"

string_id :: proc(value: string) -> string {
	return value
}

main :: proc() {
	string_id(message)
}
