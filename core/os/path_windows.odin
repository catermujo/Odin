#+private
package os

import "base:runtime"
import "core:strings"
import win32 "core:sys/windows"

_Path_Separator        :: '\\'
_Path_Separator_String :: "\\"
_Path_List_Separator   :: ';'

_is_path_separator :: proc(c: byte) -> bool {
	return c == '\\' || c == '/'
}

_mkdir :: proc(name: string, perm: Permissions) -> Error {
	temp_allocator := TEMP_ALLOCATOR_GUARD({})
	if !win32.CreateDirectoryW(_fix_long_path(name, temp_allocator) or_return, nil) {
		return _get_platform_error()
	}
	return nil
}

_mkdir_all :: proc(path: string, perm: Permissions) -> Error {
	fix_root_directory :: proc(p: string) -> (s: string, allocated: bool, err: runtime.Allocator_Error) {
		if len(p) == len(`\\?\c:`) {
			if is_path_separator(p[0]) && is_path_separator(p[1]) && p[2] == '?' && is_path_separator(p[3]) && p[5] == ':' {
				s = concatenate({p, `\`}, file_allocator()) or_return
				allocated = true
				return
			}
		}
		return p, false, nil
	}

	temp_allocator := TEMP_ALLOCATOR_GUARD({})

	dir_stat, err := stat(path, temp_allocator)
	if err == nil {
		if dir_stat.type == .Directory {
			return nil
		}
		return .Exist
	}

	i := len(path)
	for i > 0 && is_path_separator(path[i-1]) {
		i -= 1
	}

	j := i
	for j > 0 && !is_path_separator(path[j-1]) {
		j -= 1
	}

	if j > 1 {
		new_path, allocated := fix_root_directory(path[:j-1]) or_return
		defer if allocated {
			delete(new_path, file_allocator())
		}
		mkdir_all(new_path, perm) or_return
	}

	err = mkdir(path, perm)
	if err != nil {
		new_dir_stat, err1 := lstat(path, temp_allocator)
		if err1 == nil && new_dir_stat.type == .Directory {
			return nil
		}
		return err
	}
	return nil
}

_remove_all :: proc(path: string) -> Error {
	if path == "" {
		return nil
	}

	err := remove(path)
	if err == nil || err == .Not_Exist {
		return nil
	}

	temp_allocator := TEMP_ALLOCATOR_GUARD({})

	// SHFileOperationW is documented as not thread safe with relative paths.
	abs_path := path
	if !_is_absolute_path(path) {
		abs_path = _get_absolute_path(path, temp_allocator) or_return
	}

	// NOTE: The path `dir` must be double-null-terminated (`PCZZWSTR`).
	dir := win32_utf8_to_pczzwstr(abs_path, temp_allocator) or_return

	file_op := win32.SHFILEOPSTRUCTW {
		nil,
		win32.FO_DELETE,
		dir,
		nil,
		win32.FOF_NOCONFIRMATION | win32.FOF_NOERRORUI | win32.FOF_SILENT,
		false,
		nil,
		nil,
	}
	res := win32.SHFileOperationW(&file_op)
	if res != 0 {
		return _get_platform_error()
	}
	return nil
}

@private cwd_lock: win32.SRWLOCK // zero is initialized

_get_working_directory :: proc(allocator: runtime.Allocator) -> (dir: string, err: Error) {
	win32.AcquireSRWLockExclusive(&cwd_lock)

	temp_allocator := TEMP_ALLOCATOR_GUARD({ allocator })

	sz_utf16 := win32.GetCurrentDirectoryW(0, nil)
	dir_buf_wstr := make([]u16, sz_utf16, temp_allocator) or_return

	sz_utf16 = win32.GetCurrentDirectoryW(win32.DWORD(len(dir_buf_wstr)), raw_data(dir_buf_wstr))
	assert(int(sz_utf16)+1 == len(dir_buf_wstr)) // the second time, it _excludes_ the NUL.

	win32.ReleaseSRWLockExclusive(&cwd_lock)

	return win32_utf16_to_utf8(dir_buf_wstr, allocator)
}

_set_working_directory :: proc(dir: string) -> (err: Error) {
	temp_allocator := TEMP_ALLOCATOR_GUARD({})
	wstr := win32_utf8_to_wstring(dir, temp_allocator) or_return

	win32.AcquireSRWLockExclusive(&cwd_lock)

	if !win32.SetCurrentDirectoryW(wstr) {
		err = _get_platform_error()
	}

	win32.ReleaseSRWLockExclusive(&cwd_lock)

	return
}

_get_executable_path :: proc(allocator: runtime.Allocator) -> (path: string, err: Error) {
	temp_allocator := TEMP_ALLOCATOR_GUARD({ allocator })

	buf := make([dynamic]u16, 512, temp_allocator) or_return
	for {
		ret := win32.GetModuleFileNameW(nil, raw_data(buf), win32.DWORD(len(buf)))
		if ret == 0 {
			err = _get_platform_error()
			return
		}

		if ret == win32.DWORD(len(buf)) && win32.GetLastError() == win32.ERROR_INSUFFICIENT_BUFFER {
			resize(&buf, len(buf)*2) or_return
			continue
		}

		return win32_utf16_to_utf8(buf[:ret], allocator)
	}
}


@(require_results)
_fix_long_path_slice :: proc(path: string, allocator: runtime.Allocator) -> (converted: []u16, err: Error) {
	temp_allocator := TEMP_ALLOCATOR_GUARD({allocator})
	native_path := _fix_long_path_internal(path, temp_allocator) or_return
	result, allocation_error := win32_utf8_to_utf16(native_path, allocator)
	return result, allocation_error
}

@(require_results)
_fix_long_path :: proc(path: string, allocator: runtime.Allocator) -> (converted: win32.wstring, err: Error) {
	temp_allocator := TEMP_ALLOCATOR_GUARD({allocator})
	native_path := _fix_long_path_internal(path, temp_allocator) or_return
	result, allocation_error := win32_utf8_to_wstring(native_path, allocator)
	return result, allocation_error
}

@(require_results)
_fix_long_path_internal :: proc(path: string, allocator: runtime.Allocator) -> (converted: string, err: Error) {
	// Registry policy alone does not make this executable long-path aware; use explicit native paths.
	// When creating directories, MAX_PATH also reserves twelve characters for an 8.3 child name.
	if len(path) < 248 {
		return path, nil
	}
	// Explicit device paths already carry their native namespace and retain the caller's spelling.
	if strings.has_prefix(path, `\\?\`) || strings.has_prefix(path, `\\.\`) {
		return path, nil
	}
	// Relative and current-drive-rooted names need their drive and dot segments resolved before prefixing.
	absolute := _get_absolute_path(path, allocator) or_return
	defer delete(absolute, allocator)
	prefix, tail := `\\?\`, absolute
	if len(absolute) >= 2 && is_path_separator(absolute[0]) && is_path_separator(absolute[1]) {
		prefix, tail = `\\?\UNC\`, absolute[2:]
	}
	result, allocation_error := strings.concatenate({prefix, tail}, allocator)
	return result, allocation_error
}

_are_paths_identical :: strings.equal_fold

_clean_path_handle_start :: proc(path: string, buffer: []u8) -> (rooted: bool, start: int) {
	// Preserve rooted paths.
	start = _volume_name_len(path)
	if start > 0 {
		rooted = true
		if len(path) > start && _is_path_separator(path[start]) {
			// Take `C:` to `C:\`.
			start += 1
		}
		copy(buffer, path[:start])
		for n in 0..<start {
			if _is_path_separator(buffer[n]) {
				buffer[n] = _Path_Separator
			}
		}
	}
	return
}

_is_absolute_path :: proc(path: string) -> bool {
	if _is_reserved_name(path) {
		return true
	}
	if len(path) > 0 && _is_path_separator(path[0]) {
		return true
	}

	l := _volume_name_len(path)
	if l == 0 {
		return false
	}

	path := path
	path = path[l:]
	if path == "" {
		return false
	}
	return _is_path_separator(path[0])
}

_get_absolute_path :: proc(path: string, allocator: runtime.Allocator) -> (absolute_path: string, err: Error) {
	rel := path
	if rel == "" {
		rel = "."
	}
	temp_allocator := TEMP_ALLOCATOR_GUARD({ allocator })
	rel_utf16 := win32.utf8_to_utf16(rel, temp_allocator)
	n := win32.GetFullPathNameW(cstring16(raw_data(rel_utf16)), 0, nil, nil)
	if n == 0 {
		return "", _get_platform_error()
	}

	buf := make([]u16, n, temp_allocator) or_return
	n = win32.GetFullPathNameW(cstring16(raw_data(rel_utf16)), u32(n), cstring16(raw_data(buf)), nil)
	if n == 0 {
		return "", _get_platform_error()
	}

	return win32.utf16_to_utf8(buf, allocator)
}

_get_relative_path_handle_start :: proc(base, target: string) -> bool {
	base_root   := base[:_volume_name_len(base)]
	target_root := target[:_volume_name_len(target)]
	return strings.equal_fold(base_root, target_root)
}

_get_common_path_len :: proc(base, target: string) -> int {
	i := 0
	end := min(len(base), len(target))
	for j in 0..=end {
		if j == end || _is_path_separator(base[j]) {
			if i < end && _is_path_separator(base[i]) && _is_path_separator(target[i]) {
				i += 1
			}
			if strings.equal_fold(base[i:j], target[i:j]) {
				i = j
			} else {
				break
			}
		}
	}
	return i
}

_split_path :: proc(path: string) -> (dir, file: string) {
	vol_len := _volume_name_len(path)

	i := len(path) - 1
	for i >= vol_len && !_is_path_separator(path[i]) {
		i -= 1
	}
	if i == vol_len {
		return path[:i+1], path[i+1:]
	} else if i > vol_len {
		return path[:i], path[i+1:]
	}
	return "", path
}