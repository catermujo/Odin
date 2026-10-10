package test_issues

import "base:intrinsics"
import "core:testing"

@(objc_class="OdinAuditMissingClass")
Object :: struct {
	using _: intrinsics.objc_object,
}

@(test)
test_objc_message_dependencies :: proc(t: ^testing.T) {
	testing.expect_value(t, intrinsics.objc_send(int, (^Object)(nil), "hash"), 0)
	testing.expect_value(t, intrinsics.objc_send(int, Object, "hash"), 0)
}
