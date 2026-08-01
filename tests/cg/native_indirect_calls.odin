package native_indirect_calls

Int_Proc :: proc "contextless" (value: i64) -> i64
Float_Proc :: proc "c" (value: f64) -> f64
Eight_Proc :: proc "contextless" (a, b, c, d, e, f, g, h: i64) -> i64
Foreign_Proc :: proc "c" (value: i64) -> i64
Context_Proc :: proc (value: i64) -> i64
Nine_Float_Proc :: proc "c" (a, b, c, d, e, f, g, h, i: f64) -> f64

Small :: struct {
	left: u64,
	right: u64,
}

Small_Proc :: proc "contextless" (value: Small) -> Small

native_indirect_int :: proc "contextless" (value: i64) -> i64 {
	return value + 7
}

native_indirect_int_other :: proc "contextless" (value: i64) -> i64 {
	return value - 3
}

native_indirect_float :: proc "c" (value: f64) -> f64 {
	return value + 1.5
}

native_indirect_eight :: proc "contextless" (a, b, c, d, e, f, g, h: i64) -> i64 {
	return a + h
}

native_indirect_context :: proc (value: i64) -> i64 {
	return value + i64(context.user_index)
}

native_indirect_nine_float :: proc "c" (a, b, c, d, e, f, g, h, i: f64) -> f64 {
	return a + i
}

native_indirect_small :: proc "contextless" (value: Small) -> Small {
	return Small{value.left + 1, value.right + 2}
}

foreign {
	native_indirect_foreign :: proc "c" (value: i64) -> i64 ---
}

native_indirect_global_target: Int_Proc = native_indirect_int

@(export)
native_indirect_param :: proc "contextless" (target: Int_Proc, value: i64) -> i64 {
	return target(value)
}

@(export)
native_indirect_local :: proc "contextless" (value: i64) -> i64 {
	target: Int_Proc = native_indirect_int
	return target(value)
}

@(export)
native_indirect_global :: proc "contextless" (value: i64) -> i64 {
	return native_indirect_global_target(value)
}

@(export)
native_indirect_explicit :: proc "contextless" (value: i64) -> i64 {
	target: Int_Proc = native_indirect_int
	return target(value)
}

@(export)
native_indirect_foreign_call :: proc "contextless" (value: i64) -> i64 {
	target: Foreign_Proc = native_indirect_foreign
	return target(value)
}

@(export)
native_indirect_float_call :: proc "contextless" (value: f64) -> f64 {
	target: Float_Proc = native_indirect_float
	return target(value)
}

@(export)
native_indirect_eight_call :: proc "contextless" (value: i64) -> i64 {
	target: Eight_Proc = native_indirect_eight
	return target(1, 2, 3, 4, 5, 6, 7, value)
}

@(export)
native_indirect_context_call :: proc (value: i64) -> i64 {
	target: Context_Proc = native_indirect_context
	return target(value)
}

@(export)
native_indirect_nine_float_call :: proc "c" (value: f64) -> f64 {
	target: Nine_Float_Proc = native_indirect_nine_float
	return target(1, 2, 3, 4, 5, 6, 7, 8, value)
}

@(export)
native_indirect_small_call :: proc "contextless" (value: Small) -> Small {
	target: Small_Proc = native_indirect_small
	return target(value)
}

@(export)
native_indirect_control :: proc "contextless" (choose_other: bool, value: i64) -> i64 {
	target: Int_Proc = native_indirect_int
	if choose_other {
		target = native_indirect_int_other
	}
	return target(value)
}

main :: proc() {}
