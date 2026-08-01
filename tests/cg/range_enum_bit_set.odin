package main

Mode :: enum u8 { Start = 3, Middle = 9, End = 17 }
Wide :: enum u8 { High = 200 }
Flags :: bit_set[Mode; u32]

sum_modes :: proc() -> int {
	total := 0
	for mode, index in Mode {
		total += int(mode) + index
	}
	return total
}

sum_flags :: proc(flags: Flags) -> int {
	total := 0
	for flag in flags {
		total += int(flag)
	}
	return total
}

sum_flags_reverse :: proc(flags: Flags) -> int {
	total := 0
	#reverse for flag in flags {
		total = total * 10 + int(flag)
	}
	return total
}

wide_to_int :: proc(value: Wide) -> int {
	return int(value)
}

main :: proc() {
	_ = sum_modes()
	_ = sum_flags({.Start, .End})
	_ = sum_flags_reverse({.Start, .Middle, .End})
	_ = wide_to_int(.High)
}
