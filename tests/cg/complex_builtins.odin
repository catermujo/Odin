package main

components :: proc(value: complex64) -> f32 {
	return real(value) + imag(value)
}

construct :: proc(value: f32) -> complex64 {
	return complex(value, 0)
}

construct16 :: proc(value: f16) -> complex32 {
	return complex(value, 0)
}

construct16_from_f64 :: proc(value: f64) -> complex32 {
	return complex(f16(value), 0)
}

components16 :: proc(value: complex32) -> f16 {
	return real(value) + imag(value)
}

Value :: union {
	f16,
}

components16_any :: proc(value: any) -> [2]Value {
	switch z in value {
	case complex32:
		return {real(z), imag(z)}
	}
	return {}
}

Value64 :: union {
	f32,
}

components64_any :: proc(value: any) -> [2]Value64 {
	switch z in value {
	case complex64:
		return {real(z), imag(z)}
	}
	return {}
}

quaternion_construct :: proc(value: f32) -> quaternion128 {
	return quaternion(w=value, x=0, y=0, z=0)
}

conjugate_complex :: proc(value: complex64) -> complex64 {
	return conj(value)
}

conjugate_quaternion :: proc(value: quaternion128) -> quaternion128 {
	return conj(value)
}

conjugate_array :: proc(value: [2]complex64) -> [2]complex64 {
	return conj(value)
}

conjugate_matrix :: proc(value: matrix[2,2]complex64) -> matrix[2,2]complex64 {
	return conj(value)
}

main :: proc() {
	_ = components(construct(1))
	_ = construct16(1)
	_ = construct16_from_f64(1)
	_ = components16(construct16(1))
	_ = components16_any(construct16(1))
	_ = components64_any(construct(1))
	_ = quaternion_construct(1)
	_ = conjugate_complex(construct(1))
	_ = conjugate_quaternion(quaternion_construct(1))
	_ = conjugate_array({construct(1), construct(2)})
	matrix_source: matrix[2,2]complex64
	matrix_value := conjugate_matrix(matrix_source)
	_ = matrix_value[0, 0]
}
