package main

Info :: struct {
	value: int,
}

one := Info{1}
two := Info{2}

switch_panic :: proc(value: int) -> u64 {
	result: u64
	info: ^Info
	switch value {
	case 16:
		result = u64(transmute(u16)f16(f64(value)))
		info = &one
	case 32:
		result = u64(transmute(u32)f32(f64(value)))
		info = &two
	case:
		panic("invalid")
	}
	return result + u64(info.value)
}

switch_dynamic :: proc(value: int, info: ^Info) -> int {
	switch value {
	case 1<<uint(info.value) - 1:
		return 1
	case 0:
		return 2
	case:
		return 3
	}
}

main :: proc() {
	_ = switch_panic(16)
	_ = switch_dynamic(1, &two)
}
