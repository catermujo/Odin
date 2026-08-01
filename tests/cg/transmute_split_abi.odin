package main

Raw_String :: struct {
	data: [^]u8,
	len:  int,
}

string_to_raw_len :: proc(value: string) -> int {
	raw := transmute(Raw_String)value
	return raw.len
}

string_to_cstring :: proc(value: string) -> cstring {
	raw := transmute(Raw_String)value
	return cstring(raw.data)
}

raw_to_string_len :: proc(value: Raw_String) -> int {
	text := transmute(string)value
	return len(text)
}

string_to_slice_len :: proc(value: string) -> int {
	bytes := transmute([]u8)value
	return len(bytes)
}

slice_to_string_len :: proc(value: []u8) -> int {
	text := transmute(string)value
	return len(text)
}

main :: proc() {
	text := string("odin")
	raw := transmute(Raw_String)text
	_ = string_to_raw_len(text)
	_ = string_to_cstring(text)
	_ = raw_to_string_len(raw)
	_ = string_to_slice_len(text)
	_ = slice_to_string_len(transmute([]u8)text)
}
