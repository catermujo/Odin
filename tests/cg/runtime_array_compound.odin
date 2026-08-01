package main

@(export)
slice_pair_add :: proc "contextless" (left, right: ^[]i64) {
	left^ += right^
}

@(export)
slice_scalar_mul :: proc "contextless" (left: ^[]i64, factor: i64) {
	left^ *= factor
}

@(export)
slice_scalar_or_byte :: proc "contextless" (left: ^[]u8, mask: u8) {
	left^ |= mask
}

@(export)
slice_scalar_bit_ops :: proc "contextless" (left: ^[]u64, mask: u64) {
	left^ &= mask
	left^ &~= mask
	left^ |= mask
	left^ ~= mask
}

@(export)
dynamic_pair_sub :: proc "contextless" (left, right: ^[dynamic]i64) {
	left^ -= right^
}

@(export)
dynamic_scalar_mul :: proc "contextless" (left: ^[dynamic]i64, factor: i64) {
	left^ *= factor
}

@(export)
fixed_dynamic_pair_xor :: proc "contextless" (left, right: ^[dynamic; 4]u64) {
	left^ ~= right^
}

main :: proc() {}
