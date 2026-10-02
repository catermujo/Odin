package main

import "alpha"
import "beta"
import "core:testing"

foreign {
	@(link_name="m5c_alias_target") alias_target_a :: proc "c" (value: int) -> int ---
	@(link_name="m5c_alias_target") alias_target_b :: proc "c" (value: int) -> int ---
}

Identity :: proc($T: typeid, value: T) -> T {
	return value
}

@(test)
symbol_registry_test :: proc(t: ^testing.T) {
	_ = t
	assert(alpha.same_name == 11)
	assert(beta.same_name == 29)

	assert(Identity(int, 13) == 13)
	assert(Identity(int, 17) == 17)
	assert(Identity(i32, 19) == 19)
	assert(Identity(i64, 23) == 23)

	first := "cg registry duplicate"
	second := "cg registry duplicate"
	other := "cg registry distinct"
	assert(first == second)
	assert(first != other)
}
