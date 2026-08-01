package main

Callback :: #type proc(value: i32, addend: i32) -> (i32, bool)

Stream :: struct {
	procedure: Callback,
}

increment :: proc(value: i32, addend: i32) -> (i32, bool) {
	return value + addend, true
}

query :: proc(s: Stream) -> i32 {
	if s.procedure != nil {
		value, ok := s.procedure(40, 2)
		if ok {
			return value
		}
	}
	return 0
}

main :: proc() {
	stream := Stream{increment}
	_ = query(stream)
}
