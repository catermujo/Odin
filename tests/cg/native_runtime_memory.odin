package native_runtime_memory

import "base:intrinsics"

Byte_Block :: struct {
	bytes: [8]byte,
}

Word_Block :: struct {
	words: [4]u16,
}

Dword_Block :: struct {
	words: [2]u32,
}

@(export)
native_runtime_zero_align1 :: proc "c" (dst: ^Byte_Block) {
	dst^ = {}
}

@(export)
native_runtime_copy_align1 :: proc "c" (dst, src: ^Byte_Block) {
	dst^ = src^
}

@(export)
native_runtime_zero_align2 :: proc "c" (dst: ^Word_Block) {
	dst^ = {}
}

@(export)
native_runtime_copy_align2 :: proc "c" (dst, src: ^Word_Block) {
	dst^ = src^
}

@(export)
native_runtime_zero_align4 :: proc "c" (dst: ^Dword_Block) {
	dst^ = {}
}

@(export)
native_runtime_copy_align4 :: proc "c" (dst, src: ^Dword_Block) {
	dst^ = src^
}

@(export)
native_runtime_dynamic_zero :: proc "c" (dst: ^byte, size: int) {
	intrinsics.mem_zero(dst, size)
}

@(export)
native_runtime_dynamic_copy :: proc "c" (dst, src: ^byte, size: int) {
	intrinsics.mem_copy_non_overlapping(dst, src, size)
}

@(export)
native_runtime_switch_memory :: proc "c" (tag: i32, dst, src: ^byte, size: int) {
	intrinsics.mem_zero(dst, 1)
	switch tag {
	case 0:
		intrinsics.mem_zero(dst, size)
	case 1:
		intrinsics.mem_copy_non_overlapping(dst, src, size)
	case:
		intrinsics.mem_zero(dst, 2)
	}
	intrinsics.mem_zero(dst, 1)
}

main :: proc() {}
