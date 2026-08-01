package cg

@(export)
cg_identity_f32_global: f32

@(export)
cg_identity_i32_global: i32

@(export)
cg_identity_same_type :: proc(x: f32) -> f64 {
	return f64(x) + f64(cg_identity_f32_global)
}

@(export)
cg_identity_multiple_params :: proc(a, b: f32) -> f64 {
	return f64(a) + f64(b) + f64(cg_identity_f32_global)
}

@(export)
cg_identity_integer :: proc(x: i32) -> i32 {
	return x + cg_identity_i32_global
}

main :: proc() {
}
