package native_linux_amd64_syscall_reject

import "base:intrinsics"

when #config(M29K_REJECT_KIND, 0) == 1 {
	@(export)
	native_linux_amd64_reject_bsd_syscall :: proc "c" () -> uintptr {
		result, ok := intrinsics.syscall_bsd(0)
		if ok {
			return result
		}
		return 0
	}

} else when #config(M29K_REJECT_KIND, 0) == 2 {
	@(export)
	native_linux_amd64_reject_seven_arguments :: proc "c" () -> uintptr {
		return intrinsics.syscall(0, 0, 0, 0, 0, 0, 0, 0)
	}
}

main :: proc() {}
