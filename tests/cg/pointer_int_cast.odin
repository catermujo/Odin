package main

ptr_to_uintptr :: proc(ptr: rawptr) -> uintptr {
	return uintptr(ptr)
}

uintptr_to_ptr :: proc(value: uintptr) -> ^u8 {
	return (^u8)(value)
}

main :: proc() {
	value: u8
	address := ptr_to_uintptr(&value)
	_ = uintptr_to_ptr(address)
}
