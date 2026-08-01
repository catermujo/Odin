package main

Version :: bit_field u32 {
	patch: u32 | 16,
	minor: u32 | 8,
	major: u32 | 8,
}

scalar :: proc(value, mask: u64) -> u64 {
	result := value
	result &~= mask
	return result
}

array :: proc(value, mask: [2]u32) -> [2]u32 {
	result := value
	result &~= mask
	return result
}

bit_field_value :: proc(value: Version, mask: u32) -> Version {
	result := value
	result.patch &~= mask
	return result
}

map_value :: proc(value, mask: u32) -> u32 {
	values := make(map[u32]u32)
	values[0] = value
	values[0] &~= mask
	return values[0]
}

main :: proc() {
	_ = scalar(0xff, 0x0f)
	_ = array({0xff, 0xf0}, {0x0f, 0x0f})
	_ = bit_field_value(Version(0xffffffff), 0x0f)
	_ = map_value(0xff, 0x0f)
}
