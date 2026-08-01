package native_linux_amd64_float_reject

@(link_name="native_linux_amd64_float_reject_min")
@(export)
native_linux_amd64_float_reject_min :: proc "c" (left, right: f64) -> f64 {
	return min(left, right)
}

@(link_name="native_linux_amd64_float_reject_z_f16")
@(export)
native_linux_amd64_float_reject_z_f16 :: proc "c" (value: f16) -> f16 {
	return value
}
