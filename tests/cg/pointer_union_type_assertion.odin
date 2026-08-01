package main

One :: struct {
	value: int,
}

Two :: struct {}

Value :: union {
	One,
	Two,
}

read_one :: #force_no_inline proc(value: ^Value) -> int {
	return value.(One).value
}

is_one :: #force_no_inline proc(value: ^Value) -> bool {
	_, ok := value.(One)
	return ok
}

read_any_one :: #force_no_inline proc(value: any) -> int {
	return value.(One).value
}

is_any_one :: #force_no_inline proc(value: any) -> bool {
	_, ok := value.(One)
	return ok
}

main :: proc() {
	first: Value = One{value = 42}
	second: Value = Two{}
	first_any: any = One{value = 42}
	second_any: any = Two{}

	assert(read_one(&first) == 42)
	assert(is_one(&first))
	assert(!is_one(&second))
	assert(read_any_one(first_any) == 42)
	assert(is_any_one(first_any))
	assert(!is_any_one(second_any))
}
