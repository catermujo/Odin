package main

import "base:intrinsics"

when ODIN_OS == .FreeBSD || ODIN_OS == .NetBSD || ODIN_OS == .OpenBSD {
	@(export)
	syscall_bsd_probe :: proc "c" () -> uintptr {
		result, ok := intrinsics.syscall_bsd(0)
		if ok {
			return result
		}
		return 0
	}
}

main :: proc() {}
