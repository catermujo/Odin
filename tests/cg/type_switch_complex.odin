package main

@(export)
type_switch_real_i64 :: proc(value: any) -> i64 {
	switch v in value {
	case complex64:
		if imag(v) == 0 {
			return i64(real(v))
		}
	case quaternion128:
		if imag(v) == 0 && jmag(v) == 0 && kmag(v) == 0 {
			return i64(real(v))
		}
	}
	return 0
}

main :: proc() {}
