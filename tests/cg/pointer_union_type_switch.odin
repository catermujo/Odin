package main

One :: struct {
	value: int,
}

Two :: struct {}
Three :: struct {}

Value :: union {
	One,
	Two,
	Three,
}

classify :: #force_no_inline proc(value: ^Value) -> int {
	switch v in value {
	case One:
		return v.value
	case Two, Three:
		return 20
	}
	return 30
}

bump :: #force_no_inline proc(value: ^Value) {
	switch &v in value {
	case One:
		v.value += 1
	case Two, Three:
	}
}

main :: proc() {
	one: Value = One{value = 11}
	two: Value = Two{}
	three: Value = Three{}

	assert(classify(&one) == 11)
	assert(classify(&two) == 20)
	assert(classify(&three) == 20)
	bump(&one)
	assert(classify(&one) == 12)
}
