package main

@(export)
complex_parts :: proc(value: ^complex64) -> f32 {
	return real(value^) + imag(value^)
}

@(export)
quaternion_parts :: proc(value: ^quaternion128) -> f32 {
	return real(value^) + imag(value^) + jmag(value^) + kmag(value^)
}

main :: proc() {}
