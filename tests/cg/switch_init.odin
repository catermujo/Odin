package main

switch_value :: proc(value: i32) -> i32 {
	switch tag := value; tag {
	case 0:
		return 10
	case 1:
		return 20
	case:
		return 30
	}
}

main :: proc() {
	switch_value(1)
}
