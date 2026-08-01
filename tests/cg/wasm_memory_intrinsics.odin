package main

import "base:intrinsics"

wasm_memory_intrinsics :: proc() -> int {
	when ODIN_ARCH == .wasm32 {
		pages := intrinsics.wasm_memory_size(0)
		previous := intrinsics.wasm_memory_grow(0, 0)
		value: u32
		notified := intrinsics.wasm_memory_atomic_notify32(&value, 1)
		waited := intrinsics.wasm_memory_atomic_wait32(&value, 0, 0)
		return pages + previous + int(notified) + int(waited)
	}
	return 0
}

main :: proc() {
	_ = wasm_memory_intrinsics()
}
