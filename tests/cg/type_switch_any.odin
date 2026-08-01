package main

classify :: proc(value: any) -> i32 {
	switch v in value {
	case i32:
		return v + 10
	case f32:
		return i32(v) + 20
	case:
		return 30
	}
}

classify_multi :: proc(value: any) -> i32 {
	switch v in value {
	case i32, f32:
		return 20
	case nil:
		return 30
	case:
		return 40
	}
}

@(export)
classify_ref :: proc(value: any) -> i32 {
	switch &v in value {
	case i32:
		return v + 30
	case:
		return 40
	}
}

main :: proc() {
	_ = classify(i32(1))
	_ = classify(f32(2))
	_ = classify(true)
	_ = classify_multi(i32(1))
	_ = classify_multi(f32(2))
	_ = classify_multi(nil)
	_ = classify_multi(true)
	_ = classify_ref(i32(1))
}
