#+test
#+build windows
package tests_core_os

import "core:fmt"
import "core:io"
import "core:os"
import "core:strings"
import "core:testing"
import win32 "core:sys/windows"

// DUMBAI: Keep an actual native reader open across two commits and distinguish DELETE sharing from a blocked reader.
@(test)
test_rename_atomic_windows_open_readers :: proc(t: ^testing.T) {
	root, root_error := os.make_directory_temp("", "odin-rename-readers-*", context.temp_allocator)
	testing.expect_value(t, root_error, nil)
	if root_error != nil do return
	defer os.remove_all(root)
	source := os.join_path({root, "source"}, context.temp_allocator) or_else panic("could not prepare atomic rename fixture path")
	target := os.join_path({root, "target"}, context.temp_allocator) or_else panic("could not prepare atomic rename fixture path")
	for share_delete in ([2]bool{false, true}) {
		testing.expect_value(t, os.write_entire_file(target, "old-complete"), nil)
		name := win32.utf8_to_utf16(target, context.temp_allocator)
		sharing := u32(win32.FILE_SHARE_READ | win32.FILE_SHARE_WRITE)
		if share_delete do sharing |= win32.FILE_SHARE_DELETE
		reader := win32.CreateFileW(win32.LPCWSTR(raw_data(name)), win32.GENERIC_READ, sharing,
			nil, win32.OPEN_EXISTING, win32.FILE_ATTRIBUTE_NORMAL, nil)
		testing.expect(t, reader != win32.INVALID_HANDLE_VALUE)
		if reader == win32.INVALID_HANDLE_VALUE do return
		{
			defer win32.CloseHandle(reader)
			for value, index in ([2]string{"new-complete-1", "new-complete-2"}) {
				testing.expect_value(t, os.write_entire_file(source, value), nil)
				// DUMBAI: The default API retains classic replacement semantics, even for a DELETE-sharing reader.
				if index == 0 do testing.expect(t, os.rename(source, target) != nil)
				result := os.rename_atomic(source, target)
				if share_delete {
					testing.expect_value(t, result, nil)
				} else {
					testing.expect(t, result == os.Error(io.Error.Permission_Denied))
					preserved, preserved_error := os.read_entire_file(source, context.temp_allocator)
					testing.expect_value(t, preserved_error, nil)
					testing.expect_value(t, string(preserved), value)
				}
				contents, read_error := os.read_entire_file(target, context.temp_allocator)
				testing.expect_value(t, read_error, nil)
				testing.expect_value(t, string(contents), value if share_delete else "old-complete")
				testing.expect_value(t, win32.SetFilePointer(reader, 0, nil, win32.FILE_BEGIN), u32(0))
				buffer: [64]byte
				count: u32
				testing.expect(t, bool(win32.ReadFile(reader, &buffer[0], u32(len(buffer)), &count, nil)))
				testing.expect_value(t, string(buffer[:count]), "old-complete")
				if !share_delete do break
			}
		}
	}
}

// DUMBAI: Relative destinations must be fully resolved before the native rename, including UTF-16 and long paths.
@(test)
test_rename_atomic_windows_relative_long_paths :: proc(t: ^testing.T) {
	current, current_error := os.get_working_directory(context.temp_allocator)
	testing.expect_value(t, current_error, nil)
	if current_error != nil do return
	root, root_error := os.make_directory_temp(current, "odin-atomic-path-*", context.temp_allocator)
	testing.expect_value(t, root_error, nil)
	if root_error != nil do return
	defer os.remove_all(root)
	long_directory := fmt.tprintf("%s\\%s\\%s\\%s", root,
		strings.repeat("directory", 9, context.temp_allocator),
		strings.repeat("directory", 9, context.temp_allocator),
		strings.repeat("directory", 9, context.temp_allocator))
	testing.expect_value(t, os.mkdir_all(long_directory), nil)
	for directory in ([2]string{root, long_directory}) {
		source := os.join_path({directory, "源.tmp"}, context.temp_allocator) or_else panic("could not prepare atomic rename fixture path")
		target := os.join_path({directory, "租约.txt"}, context.temp_allocator) or_else panic("could not prepare atomic rename fixture path")
		relative := os.get_relative_path(current, target, context.temp_allocator) or_else panic("could not prepare atomic rename fixture path")
		testing.expect_value(t, os.write_entire_file(source, "unicode-complete"), nil)
		testing.expect_value(t, os.rename_atomic(source, relative), nil)
		contents, read_error := os.read_entire_file(target, context.temp_allocator)
		testing.expect_value(t, read_error, nil)
		testing.expect_value(t, string(contents), "unicode-complete")
		// DUMBAI: Already native-prefixed paths keep their drive namespace through the replacement path too.
		testing.expect_value(t, os.write_entire_file(source, "prefixed-complete"), nil)
		testing.expect_value(t, os.rename_atomic(fmt.tprintf(`\\?\%s`, source), fmt.tprintf(`\\?\%s`, target)), nil)
		contents, read_error = os.read_entire_file(target, context.temp_allocator)
		testing.expect_value(t, read_error, nil)
		testing.expect_value(t, string(contents), "prefixed-complete")
	}
}
