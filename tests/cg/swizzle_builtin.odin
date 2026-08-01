package main

import "base:intrinsics"

seed :: proc "contextless" (value: i32) -> i32 {
	return value
}

result: i32

main :: proc() {
	array := [4]i32{seed(10), seed(20), seed(30), seed(40)}
	array_swizzle := swizzle(array, 3, 1, 3)
	vector := #simd[4]i32{seed(10), seed(20), seed(30), seed(40)}
	vector_swizzle := swizzle(vector, 3, 1, 3, 1)
	result = array_swizzle[0] + array_swizzle[1] + array_swizzle[2] +
		intrinsics.simd_extract(vector_swizzle, 0) + intrinsics.simd_extract(vector_swizzle, 1) +
		intrinsics.simd_extract(vector_swizzle, 2) + intrinsics.simd_extract(vector_swizzle, 3)
}
