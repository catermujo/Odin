package cg_scheduler_repro

import "base:intrinsics"
import "core:simd"

U8  :: simd.u8x16
U32 :: simd.u32x4

@(export)
neon_scratch_ordering :: proc "c" (
	dst: ^u8,
	a, b: U32,
	indices: simd.u8x16,
	index: u32,
) {
	extracted := simd.extract(a, index)
	shuffled := simd.shuffle(a, b, 3, 4, 1, 7)
	swizzled := simd.runtime_swizzle(transmute(U8)shuffled, indices)
	result := simd.replace(transmute(U32)swizzled, 0, extracted)
	intrinsics.unaligned_store((^U32)(dst), result)
}
