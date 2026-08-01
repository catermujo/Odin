package main

index_array :: proc(values: [4]u8, index: int) -> u8 {
	return values[index]
}

index_slice :: proc(values: []u8, index: int) -> u8 {
	return values[index]
}

index_dynamic :: proc(values: [dynamic]u8, index: int) -> u8 {
	return values[index]
}

index_slice_pointer :: proc(values: ^[]u8, index: int) -> u8 {
	return values[index]
}

index_dynamic_pointer :: proc(values: ^[dynamic]u8, index: int) -> u8 {
	return values[index]
}

index_string :: proc(values: string, index: int) -> u8 {
	return values[index]
}

index_string_pointer :: proc(values: ^string, index: int) -> u8 {
	return values[index]
}

index_unchecked :: proc(values: []u8, index: int) -> u8 #no_bounds_check {
	return values[index]
}

index_block_unchecked :: proc(values: []u8, index: int) -> u8 {
	#no_bounds_check {
		return values[index]
	}
}

main :: proc() {
	array: [4]u8
	dynamic_values: [dynamic]u8
	slice := array[:]
	text := ""
	_ = index_array(array, 0)
	_ = index_slice(slice, 0)
	_ = index_dynamic(dynamic_values, 0)
	_ = index_slice_pointer(&slice, 0)
	_ = index_dynamic_pointer(&dynamic_values, 0)
	_ = index_string(text, 0)
	_ = index_string_pointer(&text, 0)
	_ = index_unchecked(slice, 0)
	_ = index_block_unchecked(slice, 0)
}
