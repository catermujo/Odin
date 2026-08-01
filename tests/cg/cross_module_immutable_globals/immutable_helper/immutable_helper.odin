package immutable_helper

import "base:intrinsics"

shared_literal :: proc() -> string {
	return "M6E shared immutable literal"
}

utf16_checksum :: proc() -> int {
	value := intrinsics.constant_utf16_cstring("Aé😀")
	return int(value[0]) + int(value[1]) + int(value[2]) + int(value[3]) + int(value[4])
}

check :: proc(value: string, loc := #caller_location) -> bool {
	return value == shared_literal() && value == "M6E shared immutable literal" &&
		len(loc.file_path) > 0 && loc.line > 0
}
