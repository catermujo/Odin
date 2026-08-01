package main

make_any :: proc(data: rawptr, id: typeid) -> any {
	return {id = id, data = data}
}

empty_fixed_dynamic :: proc() -> [dynamic; 4]u8 {
	return {}
}

main :: proc() {
	_ = make_any(nil, typeid_of(u8))
	_ = empty_fixed_dynamic()
}
