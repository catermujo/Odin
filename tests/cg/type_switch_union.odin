package main

Value :: union {i32, f32, bool}

classify :: proc(value: Value) -> i32 {
	#partial switch v in value {
	case i32:
		return v + 10
	case f32, bool:
		return 20
	case nil:
		return 30
	}
	return 40
}

mutate :: proc(value: ^Value) {
	#partial switch &v in value^ {
	case i32:
		v = 2
	}
}

main :: proc() {
	value: Value
	_ = classify(value)
	mutate(&value)
}
