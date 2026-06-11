package test_internal

import "core:testing"

Fast_Backend_Exported_C_Pair :: struct {
	x, y: int
}

@(export)
fast_backend_exported_c_pair_make :: proc "c" () -> Fast_Backend_Exported_C_Pair {
	return Fast_Backend_Exported_C_Pair{1, 2}
}

foreign _ {
	@(link_name="fast_backend_exported_c_pair_make")
	fast_backend_exported_c_pair_make_foreign :: proc "c" () -> Fast_Backend_Exported_C_Pair ---
}

@test
test_exported_c_aggregate_return :: proc(t: ^testing.T) {
	pair := fast_backend_exported_c_pair_make_foreign()
	testing.expect_value(t, pair.x, 1)
	testing.expect_value(t, pair.y, 2)
}
