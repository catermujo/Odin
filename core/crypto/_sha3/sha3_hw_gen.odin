#+build !arm64
package _sha3

IS_HARDWARE_ACCELERATED_SHA3 :: false

keccak_permute_hw :: proc "contextless" (st: ^[25]u64, starting_round: i32 = 0) {
	panic_contextless("crypto/sha3: hardware implementation unsupported")
}
