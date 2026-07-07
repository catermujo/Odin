#+build !js
package compress_gzip

import "core:bytes"
import "core:os"
import "core:compress/deflate"

load :: proc{load_from_file, load_from_bytes, load_from_context}

@(deprecated="Use `os.read_entire_file` and `deflate.inflate_gzip` from `core:compress/deflate`")
load_from_file :: proc(filename: string, output: ^bytes.Buffer, expected_output_size := -1, allocator := context.allocator) -> (err: Error) {
	context.allocator = allocator

	file_data, file_err := os.read_entire_file(filename, allocator)
	defer delete(file_data)

	if file_err != nil {
		return E_General.File_Not_Found
	}
	return deflate.inflate_gzip_from_bytes(&output.buf, file_data, expected_output_size, allocator=allocator)
}
