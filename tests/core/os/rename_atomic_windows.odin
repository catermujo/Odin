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


// DUMBAI: Observe the entry itself, including a dangling reparse point, without following its referent.
@(private)
rename_atomic_entry_id :: proc(t: ^testing.T, path: string) -> win32.FILE_ID_INFO {
	name := win32.utf8_to_utf16(path, context.temp_allocator)
	handle := win32.CreateFileW(win32.LPCWSTR(raw_data(name)), 0,
		win32.FILE_SHARE_READ | win32.FILE_SHARE_WRITE | win32.FILE_SHARE_DELETE,
		nil, win32.OPEN_EXISTING, win32.FILE_FLAG_OPEN_REPARSE_POINT | win32.FILE_FLAG_BACKUP_SEMANTICS, nil)
	if !testing.expect(t, handle != win32.INVALID_HANDLE_VALUE) do return {}
	defer win32.CloseHandle(handle)
	info: win32.FILE_ID_INFO
	testing.expect(t, bool(win32.GetFileInformationByHandleEx(handle, .FileIdInfo, &info, size_of(info))))
	return info
}

// DUMBAI: A directory source moves as one entry with its children; an existing directory destination stays rejected.
@(test)
test_rename_atomic_windows_directory_entry :: proc(t: ^testing.T) {
	root := os.make_directory_temp("", "odin-atomic-directory-*", context.temp_allocator) or_else
		panic("could not create directory rename fixture")
	defer os.remove_all(root)
	source := fmt.tprintf("%s/source", root)
	target := fmt.tprintf("%s/target", root)
	blocked := fmt.tprintf("%s/blocked", root)
	testing.expect_value(t, os.make_directory(source), nil)
	testing.expect_value(t, os.write_entire_file(fmt.tprintf("%s/child", source), "source-child"), nil)
	identity := rename_atomic_entry_id(t, source)
	testing.expect_value(t, os.rename_atomic(source, target), nil)
	testing.expect_value(t, rename_atomic_entry_id(t, target), identity)
	contents, read_error := os.read_entire_file(fmt.tprintf("%s/child", target), context.temp_allocator)
	testing.expect_value(t, read_error, nil)
	testing.expect_value(t, string(contents), "source-child")
	testing.expect_value(t, os.make_directory(blocked), nil)
	testing.expect_value(t, os.write_entire_file(fmt.tprintf("%s/child", blocked), "destination-child"), nil)
	testing.expect(t, os.rename_atomic(target, blocked) != nil)
	testing.expect_value(t, rename_atomic_entry_id(t, target), identity)
	contents, read_error = os.read_entire_file(fmt.tprintf("%s/child", blocked), context.temp_allocator)
	testing.expect_value(t, read_error, nil)
	testing.expect_value(t, string(contents), "destination-child")
}

// DUMBAI: Symlink creation is opt-in because it needs existing privilege or Developer Mode; absence must not fake a pass.
when #config(TEST_WINDOWS_SYMLINK_RENAME, false) {
	@(test)
	test_rename_atomic_windows_symlink_entries :: proc(t: ^testing.T) {
		root := os.make_directory_temp("", "odin-atomic-symlink-*", context.temp_allocator) or_else
			panic("could not create symlink rename fixture")
		defer os.remove_all(root)
		for label, index in ([3]string{"file", "dangling", "directory"}) {
			referent := fmt.tprintf("%s/%s-referent", root, label)
			source := fmt.tprintf("%s/%s-link", root, label)
			target := fmt.tprintf("%s/%s-moved", root, label)
			if index == 0 do testing.expect_value(t, os.write_entire_file(referent, "referent-complete"), nil)
			if index == 2 {
				testing.expect_value(t, os.make_directory(referent), nil)
				testing.expect_value(t, os.write_entire_file(fmt.tprintf("%s/child", referent), "referent-child"), nil)
			}
			original_referent: win32.FILE_ID_INFO
			if index != 1 do original_referent = rename_atomic_entry_id(t, referent)
			source_name := win32.utf8_to_utf16(source, context.temp_allocator)
			target_name := win32.utf8_to_utf16(target, context.temp_allocator)
			referent_name := win32.utf8_to_utf16(referent, context.temp_allocator)
			flags := win32.SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
			if index == 2 do flags |= win32.SYMBOLIC_LINK_FLAG_DIRECTORY
			created := win32.CreateSymbolicLinkW(win32.LPCWSTR(raw_data(source_name)),
				win32.LPCWSTR(raw_data(referent_name)), flags)
			creation_error := win32.GetLastError()
			if !testing.expect(t, bool(created), fmt.tprintf("symlink fixture unavailable: raw Win32 %d", creation_error)) {
				return
			}
			identity := rename_atomic_entry_id(t, source)
			testing.expect_value(t, os.rename_atomic(source, target), nil)
			testing.expect_value(t, rename_atomic_entry_id(t, target), identity)
			attributes := win32.GetFileAttributesW(win32.LPCWSTR(raw_data(target_name)))
			testing.expect(t, attributes != win32.INVALID_FILE_ATTRIBUTES && attributes & win32.FILE_ATTRIBUTE_REPARSE_POINT != 0)
			testing.expect_value(t, win32.GetFileAttributesW(win32.LPCWSTR(raw_data(source_name))), win32.INVALID_FILE_ATTRIBUTES)
			if index == 1 {
				testing.expect_value(t, win32.GetFileAttributesW(win32.LPCWSTR(raw_data(referent_name))), win32.INVALID_FILE_ATTRIBUTES)
			} else {
				testing.expect_value(t, rename_atomic_entry_id(t, referent), original_referent)
				read_path := referent if index == 0 else fmt.tprintf("%s/child", referent)
				contents, read_error := os.read_entire_file(read_path, context.temp_allocator)
				testing.expect_value(t, read_error, nil)
				testing.expect_value(t, string(contents), "referent-complete" if index == 0 else "referent-child")
			}
		}
	}
}
