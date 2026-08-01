package main

cleanup :: proc(value: ^i32) {
	_ = value
}

defer_in_type_switch :: proc(value: any) {
	switch item in value {
	case i32:
		local := item
		defer cleanup(&local)
	}
}

main :: proc() {
	defer_in_type_switch(i32(1))
}
