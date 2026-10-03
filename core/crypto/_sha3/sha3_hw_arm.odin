#+build arm64
package _sha3

import "base:intrinsics"
import "core:math/bits"
import "core:simd"
import "core:simd/arm"

IS_HARDWARE_ACCELERATED_SHA3 :: intrinsics.has_target_feature("neon,sha3")

@(private = "file", require_results, enable_target_feature = "neon,sha3")
xor5 :: #force_inline proc "contextless" (a, b, c, d, e: simd.u64x2) -> simd.u64x2 {
	return simd.bit_xor(arm.veor3q_u64(a, b, c), simd.bit_xor(d, e))
}

@(private = "file", require_results, enable_target_feature = "neon,sha3")
rol1 :: #force_inline proc "contextless" (a: simd.u64x2) -> simd.u64x2 {
	return arm.vxarq_u64(a, simd.u64x2{}, 63)
}

@(private = "file", require_results, enable_target_feature = "neon,sha3")
chi :: #force_inline proc "contextless" (x, y, z: simd.u64x2) -> simd.u64x2 {
	// BCAX(a,b,c) = a ^ (b & ~c). Swap the operands to form
	// x ^ (~y & z), the Keccak chi operation.
	return arm.vbcaxq_u64(x, z, y)
}

@(enable_target_feature = "neon,sha3")
keccak_permute_hw :: proc "contextless" (st: ^[25]u64, starting_round: i32 = 0) {
	ensure_contextless((starting_round >= 0 && starting_round <= 23), "crypto/sha3: invalid Keccak permutation starting round")

	for r := starting_round; r < ROUNDS; r += 1 {
		c01 := xor5(
			simd.u64x2{st[0], st[1]},
			simd.u64x2{st[5], st[6]},
			simd.u64x2{st[10], st[11]},
			simd.u64x2{st[15], st[16]},
			simd.u64x2{st[20], st[21]},
		)
		c23 := xor5(
			simd.u64x2{st[2], st[3]},
			simd.u64x2{st[7], st[8]},
			simd.u64x2{st[12], st[13]},
			simd.u64x2{st[17], st[18]},
			simd.u64x2{st[22], st[23]},
		)
		c4 := st[4] ~ st[9] ~ st[14] ~ st[19] ~ st[24]

		d01 := simd.bit_xor(simd.u64x2{c4, simd.extract(c01, 0)}, rol1(simd.u64x2{simd.extract(c01, 1), simd.extract(c23, 0)}))
		d23 := simd.bit_xor(simd.u64x2{simd.extract(c01, 1), simd.extract(c23, 0)}, rol1(simd.u64x2{simd.extract(c23, 1), c4}))
		d4 := simd.extract(c23, 1) ~ bits.rotate_left64(simd.extract(c01, 0), 1)

		for j := 0; j < 25; j += 5 {
			v01 := simd.bit_xor(simd.u64x2{st[j], st[j+1]}, d01)
			v23 := simd.bit_xor(simd.u64x2{st[j+2], st[j+3]}, d23)
			st[j], st[j+1] = simd.extract(v01, 0), simd.extract(v01, 1)
			st[j+2], st[j+3] = simd.extract(v23, 0), simd.extract(v23, 1)
			st[j+4] ~= d4
		}

		t := st[1]
		for i := 0; i < 24; i += 1 {
			j := keccak_piln[i]
			bc := st[j]
			st[j] = bits.rotate_left64(t, keccak_rotc[i])
			t = bc
		}

		for j := 0; j < 25; j += 5 {
			x0, x1, x4 := st[j], st[j+1], st[j+4]
			v0 := chi(simd.u64x2{st[j], st[j+1]}, simd.u64x2{st[j+1], st[j+2]}, simd.u64x2{st[j+2], st[j+3]})
			v1 := chi(simd.u64x2{st[j+2], st[j+3]}, simd.u64x2{st[j+3], st[j+4]}, simd.u64x2{st[j+4], st[j]})
			st[j], st[j+1] = simd.extract(v0, 0), simd.extract(v0, 1)
			st[j+2], st[j+3] = simd.extract(v1, 0), simd.extract(v1, 1)
			st[j+4] = x4 ~ (~x0 & x1)
		}

		st[0] ~= keccak_rndc[r]
	}
}
