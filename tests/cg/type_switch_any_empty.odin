package main

Empty :: struct {}

@(export)
classify :: proc(value: any) -> i32 {
	switch _ in value {
	case Empty:
		return 10
	case:
		return 20
	}
}

main :: proc() {
	_ = classify(Empty{})
}
