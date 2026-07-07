#+build !js
package compress_gzip

import "core:bytes"
import "core:os"

load :: proc{load_from_file, load_from_bytes, load_from_context}

load_from_file :: proc(filename: string, buf: ^bytes.Buffer, expected_output_size := -1, allocator := context.allocator) -> (err: Error) {
	context.allocator = allocator

	file_data, file_err := os.read_entire_file(filename, allocator)
	defer delete(file_data)

	return load_from_bytes(file_data, buf, len(file_data), expected_output_size) if file_err == nil else E_General.File_Not_Found
}
