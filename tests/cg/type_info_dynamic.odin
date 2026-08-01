package main

type_info_size :: proc(value: any) -> int {
	info := type_info_of(value.id)
	return int(info.size)
}

type_id_value :: proc(value: any) -> typeid {
	return value.id
}

main :: proc() {
	_ = type_info_size(i32(1))
	_ = type_info_size(f64(2))
	_ = type_id_value(i32(1))
}
