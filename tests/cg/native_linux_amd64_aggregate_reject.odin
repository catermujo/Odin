package native_linux_amd64_aggregate_reject

import "base:intrinsics"

Native_Linux_AMD64_SSE_Pair :: #simd[2]f64

Native_Linux_AMD64_Too_Large :: struct {
	words: [33]u64,
}

when #config(M29J_REJECT_KIND, 0) == 1 {
	@(export)
	native_linux_amd64_reject_sse_aggregate :: proc "c" (value: Native_Linux_AMD64_SSE_Pair) -> Native_Linux_AMD64_SSE_Pair {
		return value
	}
} else when #config(M29J_REJECT_KIND, 0) == 2 {
	@(export)
	native_linux_amd64_reject_dynamic_copy :: proc "c" (destination, source: ^u8, count: int) {
		intrinsics.mem_copy_non_overlapping(destination, source, count)
	}
} else when #config(M29J_REJECT_KIND, 0) == 3 {
	@(export)
	native_linux_amd64_reject_dynamic_zero :: proc "c" (destination: ^u8, count: int) {
		intrinsics.mem_zero(destination, count)
	}
} else when #config(M29J_REJECT_KIND, 0) == 4 {
	@(export)
	native_linux_amd64_reject_large_copy :: proc "c" (destination, source: ^Native_Linux_AMD64_Too_Large) {
		intrinsics.mem_copy_non_overlapping(destination, source, size_of(Native_Linux_AMD64_Too_Large))
	}
} else when #config(M29J_REJECT_KIND, 0) == 5 {
	@(export)
	native_linux_amd64_reject_large_zero :: proc "c" (destination: ^Native_Linux_AMD64_Too_Large) {
		intrinsics.mem_zero(destination, size_of(Native_Linux_AMD64_Too_Large))
	}
} else when #config(M29J_REJECT_KIND, 0) == 6 {
	@(export)
	native_linux_amd64_reject_aggregate_load :: proc "c" (address: ^Native_Linux_AMD64_Too_Large) -> Native_Linux_AMD64_Too_Large {
		return address^
	}
} else when #config(M29J_REJECT_KIND, 0) == 7 {
	@(export)
	native_linux_amd64_reject_dynamic_offset :: proc "c" (address: ^u8, count: int) -> u8 {
		return (^u8)(intrinsics.ptr_offset(address, count))^
	}
} else when #config(M29J_REJECT_KIND, 0) == 8 {
	@(export)
	native_linux_amd64_reject_aggregate_store :: proc "c" (destination: ^Native_Linux_AMD64_Too_Large, value: Native_Linux_AMD64_Too_Large) {
		destination^ = value
	}
}

main :: proc() {}
