package main

Base :: struct {
	value: int,
}

Derived :: struct {
	using base: Base,
	extra:      int,
}

as_base :: proc(d: ^Derived) -> ^Base {
	return d
}

main :: proc() {
	derived := Derived{base = {value = 7}, extra = 9}
	base := as_base(&derived)
	assert(base == &derived.base)
	assert(base^.value == 7)
	base^.value = 11
	assert(derived.base.value == 11)
	assert(derived.extra == 9)
}
