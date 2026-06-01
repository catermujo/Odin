package test_overridden_proc_alias

import "core:testing"

@test
test_overridden_proc_alias_in_defer :: proc(t: ^testing.T) {
	value := 1
	defer destroy_alias(&value)
	testing.expect_value(t, value, 1)
}
