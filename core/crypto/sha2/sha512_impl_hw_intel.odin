#+build amd64
package sha2

import "base:intrinsics"
import "core:encoding/endian"
import "core:simd"
import "core:simd/x86"
import "core:sys/info"

// SHA-512 vector extensions are exposed by CPUID.07H.01H:EAX[0].
is_hardware_accelerated_512 :: proc "contextless" () -> bool {
	req_features :: info.CPU_Features{.avx2, .sha512}
	return intrinsics.has_target_feature("avx2,sha512") && info.cpu_features() >= req_features
}

@(private = "file")
K_512 :: [80]u64{
	0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
	0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
	0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
	0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
	0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
	0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
	0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
	0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
	0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
	0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
	0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
	0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
	0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
	0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
	0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
	0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
	0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
	0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
	0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
	0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817,
}

@(private, enable_target_feature="avx2,sha512")
sha512_transf_hw :: proc "contextless" (ctx: ^Context_512, data: []byte) #no_bounds_check {
	state := ctx.h
	k := K_512
	buffer := data
	for len(buffer) >= BLOCK_SIZE_512 {
		w: [80]u64
		for i := 0; i < 16; i += 1 {
			w[i] = endian.unchecked_get_u64be(buffer[i*8:])
		}

		// VSHA512MSG1/2 compute four schedule words at a time.
		for i := 16; i < 80; i += 4 {
			msg := transmute(x86.__m256i)simd.u64x4{w[i-16], w[i-15], w[i-14], w[i-13]}
			prev := transmute(x86.__m128i)simd.u64x2{w[i-12], w[i-11]}
			msg = x86._mm256_sha512msg1_epi64(msg, prev)
			msg = transmute(x86.__m256i)(simd.add(transmute(simd.u64x4)msg, simd.u64x4{w[i-7], w[i-6], w[i-5], w[i-4]}))
			msg = x86._mm256_sha512msg2_epi64(msg, transmute(x86.__m256i)simd.u64x4{0, 0, w[i-2], w[i-1]})
			w[i], w[i+1], w[i+2], w[i+3] = u64(simd.extract(msg, 0)), u64(simd.extract(msg, 1)), u64(simd.extract(msg, 2)), u64(simd.extract(msg, 3))
		}

		a, b, c, d := state[0], state[1], state[2], state[3]
		e, f, g, h := state[4], state[5], state[6], state[7]
		for i := 0; i < 80; i += 2 {
			abef := transmute(x86.__m256i)simd.u64x4{f, e, b, a}
			cdgh := transmute(x86.__m256i)simd.u64x4{h, g, d, c}
			wk := transmute(x86.__m128i)simd.u64x2{w[i] + k[i], w[i+1] + k[i+1]}
			abef = x86._mm256_sha512rnds2_epi64(abef, cdgh, wk)

			old_b, old_c, old_f, old_g := b, c, f, g
			a, b, e, f = u64(simd.extract(abef, 3)), u64(simd.extract(abef, 2)), u64(simd.extract(abef, 1)), u64(simd.extract(abef, 0))
			c, d, g, h = old_b, old_c, old_f, old_g
		}

		state[0] += a
		state[1] += b
		state[2] += c
		state[3] += d
		state[4] += e
		state[5] += f
		state[6] += g
		state[7] += h
		buffer = buffer[BLOCK_SIZE_512:]
	}

	ctx.h = state
}
