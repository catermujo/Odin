#+feature dynamic-literals

package main

Item :: struct {
	value: int,
}

fixed :: proc() -> [dynamic; 4]Item {
	return {{value = 1}, {value = 2}}
}

make_dynamic :: proc() -> [dynamic]Item {
	return {{value = 1}, {value = 2}}
}

main :: proc() {
	_ = fixed()
	_ = make_dynamic()
}
