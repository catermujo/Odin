package test_internal

import "base:intrinsics"
import "core:testing"

fast_backend_diverging_trap :: proc() -> ! {
	intrinsics.trap()
}

fast_backend_diverging_wrapper :: proc(run: bool) {
	if run {
		fast_backend_diverging_trap()
	}
}

@test
test_diverging_proc :: proc(t: ^testing.T) {
	fast_backend_diverging_wrapper(false)
	testing.expect_value(t, true, true)
}
