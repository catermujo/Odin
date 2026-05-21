#+build define:ODIN_TEST_BUILD_TAG_DEFINE
package test_issue_build_tag_define_order

trigger_error_when_enabled :: proc() {
	does_not_exist()
}
