#+test
package tests_core_os

import "core:os"
import "core:testing"

// DUMBAI: Exercise the new API's real pathname commit and rejection semantics without changing ordinary rename.
@(test)
test_rename_atomic :: proc(t: ^testing.T) {
	root, root_error := os.make_directory_temp("", "odin-rename-atomic-*", context.temp_allocator)
	testing.expect_value(t, root_error, nil)
	if root_error != nil do return
	defer os.remove_all(root)
	source := os.join_path({root, "source"}, context.temp_allocator) or_else panic("could not prepare atomic rename fixture path")
	target := os.join_path({root, "target"}, context.temp_allocator) or_else panic("could not prepare atomic rename fixture path")
	for value in ([2]string{"first", "second-complete"}) {
		testing.expect_value(t, os.write_entire_file(source, value), nil)
		testing.expect_value(t, os.rename_atomic(source, target), nil)
		contents, read_error := os.read_entire_file(target, context.temp_allocator)
		testing.expect_value(t, read_error, nil)
		testing.expect_value(t, string(contents), value)
	}
	testing.expect_value(t, os.write_entire_file(source, "preserved-source"), nil)
	testing.expect(t, os.rename_atomic(source, root) != nil)
	contents, read_error := os.read_entire_file(source, context.temp_allocator)
	testing.expect_value(t, read_error, nil)
	testing.expect_value(t, string(contents), "preserved-source")
	contents, read_error = os.read_entire_file(target, context.temp_allocator)
	testing.expect_value(t, read_error, nil)
	testing.expect_value(t, string(contents), "second-complete")
}
