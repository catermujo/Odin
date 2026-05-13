// Tests disabled procedures skip signature/body/argument checking when disabled.
package test_issues

import "core:testing"

EVAL :: false

@(private = "file")
@(disabled = !EVAL)
disabled_proc_kills_checks :: proc(x: DoesNotExist) {
	_ = missing_symbol()
}

@(test)
test_disabled_proc_kills_checks :: proc(t: ^testing.T) {
	disabled_proc_kills_checks(missing_symbol())
	testing.expect(t, true)
}
