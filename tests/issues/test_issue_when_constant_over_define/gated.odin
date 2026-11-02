#+when ODIN_TEST_WHEN_GATE
package test_issue_when_constant_over_define

trigger_error_when_enabled :: proc() {
	does_not_exist()
}
