#+private
package os

import "base:runtime"
import "core:time"
import win32 "core:sys/windows"

@(private="file")
find_data_to_file_info :: proc(base_path: string, d: ^win32.WIN32_FIND_DATAW, allocator: runtime.Allocator) -> (fi: File_Info, err: Error) {
	// Ignore "." and ".."
	if d.cFileName[0] == '.' && d.cFileName[1] == 0 {
		return
	}
	if d.cFileName[0] == '.' && d.cFileName[1] == '.' && d.cFileName[2] == 0 {
		return
	}

	temp_allocator := TEMP_ALLOCATOR_GUARD({ allocator })
	filename := win32_wstring_to_utf8(cstring16(raw_data(d.cFileName[:])), temp_allocator) or_else ""

	pieces: []string
	if len(base_path) > 0 && is_path_separator(base_path[len(base_path) - 1]) {
		pieces = {base_path, filename}
	} else {
		pieces = {base_path, Path_Separator_String, filename}
	}
	path   := concatenate(pieces, allocator) or_return
	// DUMBAI: Entry metadata must not block another worker's atomic replacement or temporary-file cleanup.
	handle := win32.HANDLE(_open_internal(path, {.Read, .Share_Delete}, Permissions_Read_Write_All) or_else 0)
	defer win32.CloseHandle(handle)

	fi.fullpath = path
	fi.name = basename(path)
	fi.size = i64(d.nFileSizeHigh)<<32 + i64(d.nFileSizeLow)

	fi.type, fi.mode = _file_type_mode_from_file_attributes(d.dwFileAttributes, handle, d.dwReserved0)

	fi.creation_time     = time.unix(0, win32.FILETIME_as_unix_nanoseconds(d.ftCreationTime))
	fi.modification_time = time.unix(0, win32.FILETIME_as_unix_nanoseconds(d.ftLastWriteTime))
	fi.access_time       = time.unix(0, win32.FILETIME_as_unix_nanoseconds(d.ftLastAccessTime))

	if file_id_info: win32.FILE_ID_INFO; handle != nil && win32.GetFileInformationByHandleEx(handle, .FileIdInfo, &file_id_info, size_of(file_id_info)) {
		#assert(size_of(fi.inode) == size_of(file_id_info.FileId))
		#assert(size_of(fi.inode) == 16)
		runtime.mem_copy_non_overlapping(&fi.inode, &file_id_info.FileId, 16)
		fi.device = file_id_info.VolumeSerialNumber
	}

	return
}

Read_Directory_Iterator_Impl :: struct {
	find_data:     win32.WIN32_FIND_DATAW,
	find_handle:   win32.HANDLE,
	path:          string,
	prev_fi:       File_Info,
	no_more_files: bool,
}


@(require_results)
_read_directory_iterator :: proc(it: ^Read_Directory_Iterator) -> (fi: File_Info, index: int, ok: bool) {
	for !it.impl.no_more_files {
		err: Error
		file_info_delete(it.impl.prev_fi, file_allocator())
		it.impl.prev_fi = {}

		fi, err = find_data_to_file_info(it.impl.path, &it.impl.find_data, file_allocator())
		if err != nil {
			read_directory_iterator_set_error(it, it.impl.path, err)
			return
		}

		if fi.name != "" {
			it.impl.prev_fi = fi
			ok = true
			index = it.index
			it.index += 1
		}

		if !win32.FindNextFileW(it.impl.find_handle, &it.impl.find_data) {
			e := _get_platform_error()
			if pe, _ := is_platform_error(e); pe != i32(win32.ERROR_NO_MORE_FILES) {
				read_directory_iterator_set_error(it, it.impl.path, e)
			}
			it.impl.no_more_files = true
		}
		if ok {
			return
		}
	}
	return
}

_read_directory_iterator_init :: proc(it: ^Read_Directory_Iterator, f: ^File) {
	it.impl.no_more_files = false

	if f == nil || f.impl == nil {
		read_directory_iterator_set_error(it, "", .Invalid_File)
		return
	}

	it.f = f
	impl := (^File_Impl)(f.impl)

	// NOTE: Allow calling `init` to target a new directory with the same iterator - reset idx.
	if it.impl.find_handle != nil {
		win32.FindClose(it.impl.find_handle)
	}
	if it.impl.path != "" {
		delete(it.impl.path, file_allocator())
	}

	if !is_directory(impl.name) {
		read_directory_iterator_set_error(it, impl.name, .Invalid_Dir)
		return
	}

	temp_allocator := TEMP_ALLOCATOR_GUARD({})
	// DUMBAI: Directory enumeration must use the same native long-path spelling as the successful open.
	search_path, allocation_error := concatenate({impl.name, `\*`}, temp_allocator)
	if allocation_error != nil {
		read_directory_iterator_set_error(it, impl.name, allocation_error)
		return
	}
	wpath_search, path_error := _fix_long_path(search_path, temp_allocator)
	if path_error != nil {
		read_directory_iterator_set_error(it, impl.name, path_error)
		return
	}

	it.impl.find_handle = win32.FindFirstFileW(wpath_search, &it.impl.find_data)
	if it.impl.find_handle == win32.INVALID_HANDLE_VALUE {
		read_directory_iterator_set_error(it, impl.name, _get_platform_error())
		return
	}
	defer if it.err.err != nil {
		win32.FindClose(it.impl.find_handle)
	}

	err: Error
	it.impl.path, err = get_absolute_path(impl.name, file_allocator())
	if err != nil {
		read_directory_iterator_set_error(it, impl.name, err)
	}

	return
}

_read_directory_iterator_destroy :: proc(it: ^Read_Directory_Iterator) {
	if it.f == nil {
		return
	}
	file_info_delete(it.impl.prev_fi, file_allocator())
	delete(it.impl.path, file_allocator())
	win32.FindClose(it.impl.find_handle)
}
