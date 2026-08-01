package main

import "base:intrinsics"

when ODIN_ARCH == .amd64 || ODIN_ARCH == .i386 {
	@(export)
	x86_cpuid_probe :: proc "c" (ax, cx: u32) -> u32 {
		eax, _, _, _ := intrinsics.x86_cpuid(ax, cx)
		return eax
	}

	@(export)
	x86_xgetbv_probe :: proc "c" (cx: u32) -> u32 {
		eax, _ := intrinsics.x86_xgetbv(cx)
		return eax
	}
}

main :: proc() {}
