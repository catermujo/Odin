package main

type_info_switch :: proc(id: typeid) -> i32 {
	switch type_info_of(id) {
	case type_info_of(u32):
		return 1
	case type_info_of(u16):
		return 2
	case:
		return 0
	}
}

main :: proc() {
	_ = type_info_switch(u32)
	_ = type_info_switch(u16)
}
