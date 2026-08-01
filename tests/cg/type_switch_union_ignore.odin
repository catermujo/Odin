package main

Value :: union {i32, f32, bool}

classify :: proc(value: Value) -> i32 {
	#partial switch _ in value {
	case i32:  return 10
	case f32:  return 20
	case bool: return 30
	}
	return 40
}

main :: proc() {
	value: Value
	_ = classify(value)
}
