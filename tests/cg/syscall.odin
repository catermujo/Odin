package cg

import "base:intrinsics"

syscall_probe :: proc(id, a, b, c, d, e, f: uintptr) -> uintptr {
	return intrinsics.syscall(id, a, b, c, d, e, f)
}

main :: proc() {
	_ = syscall_probe(0, 0, 0, 0, 0, 0, 0)
}
