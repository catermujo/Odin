#+test
package tests_core_os

import "core:fmt"
import "core:os"
import "core:log"
import "core:slice"
import "core:testing"
import "core:strings"

@(test)
test_read_dir :: proc(t: ^testing.T) {
	path, err_join := os.join_path({#directory, "dir"}, context.allocator)
	defer delete(path)

	fis, err_read := os.read_all_directory_by_path(path, context.allocator)
	defer os.file_info_slice_delete(fis, context.allocator)

	slice.sort_by_key(fis, proc(fi: os.File_Info) -> string { return fi.name })

	if err_read == .Unsupported {
		log.warn("core:os directory functionality is unsupported, skipping test")
		return
	}

	testing.expect_value(t, err_join, nil)
	testing.expect_value(t, err_read, nil)
	testing.expect_value(t, len(fis), 2)

	testing.expect_value(t, fis[0].name, "b.txt")
	testing.expect_value(t, fis[0].type, os.File_Type.Regular)

	testing.expect_value(t, fis[1].name, "sub")
	testing.expect_value(t, fis[1].type, os.File_Type.Directory)
}

@(test)
test_walker :: proc(t: ^testing.T) {
	path, err := os.join_path({#directory, "dir"}, context.allocator)
	defer delete(path)
	testing.expect_value(t, err, nil)

	w := os.walker_create(path)
	defer os.walker_destroy(&w)

	test_walker_internal(t, &w)
}

@(test)
test_walker_file :: proc(t: ^testing.T) {
	path, err_join := os.join_path({#directory, "dir"}, context.allocator)
	defer delete(path)
	testing.expect_value(t, err_join, nil)

	f, err_open := os.open(path)
	testing.expect_value(t, err_open, nil)
	defer os.close(f)

	w := os.walker_create(f)
	defer os.walker_destroy(&w)

	test_walker_internal(t, &w)
}

test_walker_internal :: proc(t: ^testing.T, w: ^os.Walker) {
	Seen :: struct {
		type: os.File_Type,
		path: string,
	}

	joined_1, err_joined_1 := os.join_path({"dir", "b.txt"}, context.allocator)
	joined_2, err_joined_2 := os.join_path({"dir", "sub"}, context.allocator)
	joined_3, err_joined_3 := os.join_path({"dir", "sub", ".gitkeep"}, context.allocator)

	testing.expect_value(t, err_joined_1, nil)
	testing.expect_value(t, err_joined_2, nil)
	testing.expect_value(t, err_joined_3, nil)

	expected := [?]Seen{
		{.Regular,   joined_1},
		{.Directory, joined_2},
		{.Regular,   joined_3},
	}

	seen: [dynamic]Seen
	defer delete(seen)

	for info in os.walker_walk(w) {

		errpath, err := os.walker_error(w)
		testing.expectf(t, err == nil, "walker error for %q: %v", errpath, err)

		append(&seen, Seen{
			info.type,
			strings.clone(info.fullpath),
		})
	}

	if _, err := os.walker_error(w); err == .Unsupported {
		log.warn("core:os directory functionality is unsupported, skipping test")
		return
	}

	testing.expect_value(t, len(seen), len(expected))

	for expectation in expected {
		found: bool
		for entry in seen {
			if strings.has_suffix(entry.path, expectation.path) {
				found = true
				testing.expect_value(t, entry.type, expectation.type)
				delete(entry.path)
			}
		}
		testing.expectf(t, found, "%q not found in %v", expectation, seen)
		delete(expectation.path)
	}
} 

// DUMBAI: Windows directory spelling is a native Win32 contract.
when ODIN_OS == .Windows {
	// DUMBAI: Enumeration must resolve the same long names that open already accepts, without manifest changes.
	@(test)
	test_directory_windows_long_path_spellings :: proc(t: ^testing.T) {
		current := os.get_working_directory(context.temp_allocator) or_else panic("could not read the fixture directory")
		root := os.make_directory_temp(current, "directory-long-*", context.temp_allocator) or_else
			panic("could not create directory fixture")
		defer { _ = os.remove(root) }
		component := strings.repeat("directory", 9, context.temp_allocator)
		first := os.join_path({root, component}, context.temp_allocator) or_else panic("could not join directory")
		second := os.join_path({first, component}, context.temp_allocator) or_else panic("could not join directory")
		third := os.join_path({second, component}, context.temp_allocator) or_else panic("could not join directory")
		short := os.join_path({root, "short"}, context.temp_allocator) or_else panic("could not join directory")
		directories := [4]string{short, first, second, third}
		created := 0
		defer {
			for created > 0 {
				created -= 1
				_ = os.remove(fmt.tprintf(`\\?\%s`, directories[created]))
			}
		}
		for directory in directories {
			native := fmt.tprintf(`\\?\%s`, directory)
			error := os.make_directory(native)
			testing.expect_value(t, error, nil)
			if error != nil do return
			created += 1
		}
		for directory in ([2]string{short, third}) {
			name := "advertisement-租约.record"
			path := os.join_path({directory, name}, context.temp_allocator) or_else panic("could not join entry")
			native_path := fmt.tprintf(`\\?\%s`, path)
			testing.expect_value(t, os.write_entire_file(native_path, transmute([]byte)string("complete advertisement")), nil)
			defer { _ = os.remove(native_path) }
			relative := os.get_relative_path(current, directory, context.temp_allocator) or_else
				panic("could not resolve relative directory")
			if directory == third {
				testing.expect(t, len(directory) > 260 && len(relative) >= 248)
			}
			for alias in ([3]string{directory, relative, fmt.tprintf(`\\?\%s`, directory)}) {
				entries, error := os.read_all_directory_by_path(alias, context.allocator)
				defer os.file_info_slice_delete(entries, context.allocator)
				testing.expect_value(t, error, nil)
				testing.expect_value(t, len(entries), 1)
				if error != nil || len(entries) != 1 do continue
				entry := entries[0]
				testing.expect_value(t, entry.name, name)
				testing.expect_value(t, entry.type, .Regular)
				expected_path := native_path if strings.has_prefix(alias, `\\?\`) else path
				testing.expect_value(t, entry.fullpath, expected_path)
				bytes, read_error := os.read_entire_file(entry.fullpath, context.temp_allocator)
				testing.expect_value(t, read_error, nil)
				testing.expect_value(t, string(bytes), "complete advertisement")
			}
		}
	}
}
