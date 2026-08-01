package reach_helper

Key :: struct {
	id:   i32,
	text: string,
}

global_value: i32 = seed_value()

seed_value :: proc "contextless" () -> i32 {
	return 13
}

transform :: proc(value: i32) -> i32 {
	return value * 2
}

make_key :: proc() -> Key {
	return Key{id = 7, text = "reach"}
}
