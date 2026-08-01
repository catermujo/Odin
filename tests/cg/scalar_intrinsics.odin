package cg

import "base:intrinsics"

fma :: proc(x, y, z: f64) -> f64 {
	return intrinsics.fused_mul_add(x, y, z)
}

debug_break :: proc() {
	intrinsics.debug_trap()
}

fatal :: proc() {
	intrinsics.trap()
}

maybe_fatal :: proc(fatal_now: bool) {
	if fatal_now {
		fatal()
	}
}

main :: proc() {
	assert(fma(2, 3, 4) == 10)
	debug_break()
	maybe_fatal(false)
}
