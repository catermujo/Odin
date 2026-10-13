// Two foreign blocks that import the same C global must share one external symbol.
// The backend renamed the second LLVM global to "name.1", so the link failed.
package test_issue_shared_foreign_global

import "core:testing"
import "a"
import "b"

@(test)
test_shared_foreign_global :: proc(t: ^testing.T) {
	when ODIN_OS == .Darwin {
		direct_a := uintptr(a.vm_page_size)
	} else {
		direct_a := uintptr(a.environ)
	}
	testing.expect(t, direct_a != 0)
	testing.expect_value(t, direct_a, uintptr(b.value))
	testing.expect_value(t, a.shared_global(), b.shared_global())
}
