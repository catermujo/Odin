package native_abi_stack_args

Large :: struct {
	first: u64,
	second: u64,
	third: u64,
	fourth: u64,
}

@(export)
native_abi_stack_integer :: proc(a, b, c, d, e, f, g, h, i, j: i64) -> i64 {
	return a + b + c + d + e + f + g + h + i + j
}

@(export)
native_abi_stack_float :: proc(a, b, c, d, e, f, g, h, i, j: f64) -> f64 {
	return a + b + c + d + e + f + g + h + i + j
}

@(export)
native_abi_stack_mixed :: proc(a, b, c, d, e, f: i64, x, y, z, w: f64, text: string, value: Large) -> f64 {
	return f64(a+b+c+d+e+f) + x + y + z + w + f64(len(text)) + f64(value.first) + f64(value.fourth)
}

main :: proc() {}
