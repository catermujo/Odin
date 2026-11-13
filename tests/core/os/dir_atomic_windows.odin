#+test
#+build windows
package tests_core_os

import "core:os"
import "core:io"
import "core:strings"
import "core:sync"
import "core:testing"
import "core:thread"
import "core:time"

Directory_Atomic_Reader :: struct {
	root: string,
	ready, stop: i32,
	scans: int,
	error: os.Error,
}

directory_atomic_reader :: proc(worker: ^thread.Thread) {
	reader := cast(^Directory_Atomic_Reader)worker.data
	defer free_all(context.temp_allocator)
	sync.atomic_store_explicit(&reader.ready, 1, .Release)
	for sync.atomic_load_explicit(&reader.stop, .Acquire) == 0 {
		entries, err := os.read_all_directory_by_path(reader.root, context.allocator)
		if err != nil { reader.error = err; break }
		reader.scans += 1
		os.file_info_slice_delete(entries, context.allocator)
	}
}

// Real metadata enumeration must coexist with complete-file publication without locking its temporary.
@(test)
test_directory_inventory_allows_atomic_publication :: proc(t: ^testing.T) {
	root, root_error := os.make_directory_temp("", "directory-atomic-*", context.temp_allocator)
	testing.expect_value(t, root_error, nil)
	if root_error != nil do return
	defer os.remove_all(root)
	target := os.join_path({root, "effects.json"}, context.temp_allocator) or_else panic("could not prepare target")
	reader := Directory_Atomic_Reader{root = root}
	worker := thread.create(directory_atomic_reader, name = "directory publication reader")
	testing.expect(t, worker != nil)
	if worker == nil do return
	worker.data = &reader
	thread.start(worker)
	for sync.atomic_load_explicit(&reader.ready, .Acquire) == 0 do time.sleep(time.Millisecond)
	publication_error: os.Error
	attempted := 0
	started := time.tick_now()
	for attempted < 2000 && time.tick_since(started) < 10*time.Second {
		attempted += 1
		file, create_error := os.create_temp_file(root, ".atomic-*.tmp")
		if create_error != nil { publication_error = create_error; break }
		source := strings.clone(os.name(file), context.temp_allocator)
		written, write_error := os.write(file, transmute([]byte)string("complete"))
		close_error := os.close(file)
		if write_error != nil { publication_error = write_error; break }
		if close_error != nil { publication_error = close_error; break }
		testing.expect_value(t, written, len("complete"))
		publication_error = os.rename_atomic(source, target)
		if publication_error != nil do break
	}
	sync.atomic_store_explicit(&reader.stop, 1, .Release)
	thread.join(worker)
	thread.destroy(worker)
	testing.expect_value(t, reader.error, nil)
	testing.expect(t, reader.scans > 0 && attempted > 0)
	testing.expect_value(t, publication_error, nil)
	contents, read_error := os.read_entire_file(target, context.temp_allocator)
	testing.expect_value(t, read_error, nil)
	testing.expect_value(t, string(contents), "complete")
}

// Opt-in snapshot readers keep old bytes, while ordinary readers retain their pathname protection.
@(test)
test_share_delete_reader_preserves_atomic_snapshot :: proc(t: ^testing.T) {
	root, root_error := os.make_directory_temp("", "shared-snapshot-*", context.temp_allocator)
	testing.expect_value(t, root_error, nil)
	if root_error != nil do return
	defer os.remove_all(root)
	source := os.join_path({root, "source"}, context.temp_allocator) or_else panic("could not prepare source")
	target := os.join_path({root, "target"}, context.temp_allocator) or_else panic("could not prepare target")
	for shared in ([2]bool{false, true}) {
		testing.expect_value(t, os.write_entire_file(target, "old-complete"), nil)
		testing.expect_value(t, os.write_entire_file(source, "new-complete"), nil)
		flags := os.File_Flags{.Read}
		if shared do flags += {.Share_Delete}
		reader, open_error := os.open(target, flags)
		testing.expect_value(t, open_error, nil)
		if open_error != nil do return
		{
			defer os.close(reader)
			rename_error := os.rename_atomic(source, target)
			if shared do testing.expect_value(t, rename_error, nil)
			else do testing.expect_value(t, rename_error, os.Error(io.Error.Permission_Denied))
			snapshot, snapshot_error := os.read_entire_file(reader, context.temp_allocator)
			testing.expect_value(t, snapshot_error, nil)
			testing.expect_value(t, string(snapshot), "old-complete")
			current, current_error := os.read_entire_file(target, context.temp_allocator)
			testing.expect_value(t, current_error, nil)
			testing.expect_value(t, string(current), "new-complete" if shared else "old-complete")
		}
	}
}
