package main

import "core:simd"

gather_i32 :: proc(ptrs: #simd[4]rawptr, val: simd.i32x4, mask: simd.u32x4) -> simd.i32x4 { return simd.gather(ptrs, val, mask) }
scatter_i32 :: proc(ptrs: #simd[4]rawptr, val: simd.i32x4, mask: simd.u32x4) { simd.scatter(ptrs, val, mask) }
masked_load_i32 :: proc(ptr: rawptr, val: simd.i32x4, mask: simd.u32x4) -> simd.i32x4 { return simd.masked_load(ptr, val, mask) }
masked_load_bool :: proc(ptr: rawptr, val: simd.i32x4, mask: #simd[4]bool) -> simd.i32x4 { return simd.masked_load(ptr, val, mask) }
masked_store_i32 :: proc(ptr: rawptr, val: simd.i32x4, mask: simd.u32x4) { simd.masked_store(ptr, val, mask) }
expand_load_i32 :: proc(ptr: rawptr, val: simd.i32x4, mask: simd.u32x4) -> simd.i32x4 { return simd.masked_expand_load(ptr, val, mask) }
compress_store_i32 :: proc(ptr: rawptr, val: simd.i32x4, mask: simd.u32x4) { simd.masked_compress_store(ptr, val, mask) }

main :: proc() {
	ptr: rawptr = nil
	ptrs: #simd[4]rawptr
	val: simd.i32x4 = 1
	mask: simd.u32x4 = 2
	_ = gather_i32(ptrs, val, mask)
	scatter_i32(ptrs, val, mask)
	_ = masked_load_i32(ptr, val, mask)
	_ = masked_load_bool(ptr, val, #simd[4]bool{true, true, true, true})
	masked_store_i32(ptr, val, mask)
	_ = expand_load_i32(ptr, val, mask)
	compress_store_i32(ptr, val, mask)
}
