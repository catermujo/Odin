#+test
package test_internal

import "core:testing"
import fixture "test_defined_imports_fixture"

FILE_SCOPE_ONLY :: 99

@(test)
defined_imported_selectors :: proc(t: ^testing.T) {
	local_only := 1
	testing.expect(t, #defined(local_only))
	testing.expect(t, !#defined(absent_local))
	testing.expect(t, #defined(fixture.imported_proc))
	testing.expect(t, #defined(fixture.IMPORTED_CONSTANT))
	testing.expect(t, !#defined(fixture.local_only))
	testing.expect(t, !#defined(fixture.FILE_SCOPE_ONLY))
	testing.expect(t, !#defined(fixture.absent_member))
	testing.expect_value(t, fixture.imported_proc(), 7)
}
