package semantic_thread_local

foreign {
	semantic_thread_local_sink :: proc "c" (value: u64) -> u64 ---
}

@(thread_local)
semantic_tls_file_default: u64

@(thread_local="globaldynamic")
semantic_tls_file_globaldynamic: u64

@(thread_local="localdynamic")
semantic_tls_file_localdynamic: u64

@(thread_local="initialexec")
semantic_tls_file_initialexec: u64

@(thread_local="localexec")
semantic_tls_file_localexec: u64

semantic_tls_file_ordinary: u64

@(export)
semantic_tls_file_access :: proc "c" (selector: i32, next: u64, condition: bool) -> u64 {
	pointer := &semantic_tls_file_default
	switch selector {
	case 1:
		pointer = &semantic_tls_file_globaldynamic
	case 2:
		pointer = &semantic_tls_file_localdynamic
	case 3:
		pointer = &semantic_tls_file_initialexec
	case 4:
		pointer = &semantic_tls_file_localexec
	case 5:
		pointer = &semantic_tls_file_ordinary
	}

	before := pointer^
	pointer^ = next
	barrier := semantic_thread_local_sink(before)
	result := pointer^
	if condition {
		result += barrier
	} else {
		result += u64(uintptr(pointer))
	}
	return result
}

@(export)
semantic_tls_local_access :: proc "c" (selector: i32, next: u64, condition: bool) -> u64 {
	@(thread_local)
	local_default: u64
	@(thread_local="globaldynamic")
	local_globaldynamic: u64
	@(thread_local="localdynamic")
	local_localdynamic: u64
	@(thread_local="initialexec")
	local_initialexec: u64
	@(thread_local="localexec")
	local_localexec: u64
	@(static)
	local_ordinary: u64

	pointer := &local_default
	switch selector {
	case 1:
		pointer = &local_globaldynamic
	case 2:
		pointer = &local_localdynamic
	case 3:
		pointer = &local_initialexec
	case 4:
		pointer = &local_localexec
	case 5:
		pointer = &local_ordinary
	}

	before := pointer^
	pointer^ = next
	barrier := semantic_thread_local_sink(before)
	result := pointer^
	if condition {
		result += barrier
	} else {
		result += u64(uintptr(pointer))
	}
	return result
}

main :: proc() {}
