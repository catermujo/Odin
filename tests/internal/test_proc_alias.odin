package test_internal

import "core:testing"

base_proc_alias_target :: proc() -> int {
	return 42
}

base_proc_alias :: base_proc_alias_target

@test
test_proc_alias :: proc(t: ^testing.T) {
	testing.expect_value(t, base_proc_alias(), 42)
}
