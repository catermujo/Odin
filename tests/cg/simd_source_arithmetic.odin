package simd_source_arithmetic

import "base:intrinsics"
import "core:simd"

F32 :: simd.f32x4

@(export)
rounding :: proc "c" (dst: ^u8, value: F32) {
	result := simd.ceil(value) +
	          simd.floor(value) +
	          simd.trunc(value) +
	          simd.nearest(value)
	intrinsics.unaligned_store((^F32)(dst), result)
}
