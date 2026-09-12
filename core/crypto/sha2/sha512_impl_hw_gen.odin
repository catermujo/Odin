#+build i386, arm32, wasm32, wasm64p32, riscv64
package sha2

// is_hardware_accelerated_512 returns true if and only if (⟺) hardware
// accelerated SHA-384, SHA-512, and SHA-512/256 are supported.
is_hardware_accelerated_512 :: proc "contextless" () -> bool {
	return false
}

@(private)
sha512_transf_hw :: proc "contextless" (ctx: ^Context_512, data: []byte) {
	panic_contextless(ERR_HW_NOT_SUPPORTED)
}
