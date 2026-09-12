#+build arm64,arm32
package aes_hw

import "core:crypto/_aes"
import "core:encoding/endian"
import "core:math/bits"
import "core:simd/arm"

// Carryless multiply of two 64-bit polynomial values.
@(private = "file", require_results, enable_target_feature = TARGET_FEATURES)
clmul64 :: #force_inline proc "contextless" (a, b: u64) -> (lo, hi: u64) {
	product := arm.vmull_p64(arm.poly64_t(a), arm.poly64_t(b))
	return u64(product), u64(product >> 64)
}

// Multiply two values in GF(2^128), using x^128 + x^7 + x^2 + x + 1.
@(private = "file", require_results, enable_target_feature = TARGET_FEATURES)
gfmul :: #force_inline proc "contextless" (x0, x1, y0, y1: u64) -> (lo, hi: u64) {
	p00_lo, p00_hi := clmul64(x0, y0)
	p01_lo, p01_hi := clmul64(x0, y1)
	p10_lo, p10_hi := clmul64(x1, y0)
	p11_lo, p11_hi := clmul64(x1, y1)

	m_lo := p01_lo ~ p10_lo
	m_hi := p01_hi ~ p10_hi
	p0 := p00_lo
	p1 := p00_hi ~ m_lo
	p2 := m_hi ~ p11_lo
	p3 := p11_hi

	// Fold the upper half with x^128 = x^7 + x^2 + x + 1.
	r0_lo, r0_hi := clmul64(p2, 0x87)
	r1_lo, r1_hi := clmul64(p3, 0x87)
	r2_lo, r2_hi := clmul64(r1_hi, 0x87)

	return p0 ~ r0_lo ~ r2_lo, p1 ~ r0_hi ~ r1_lo ~ r2_hi
}

// ghash calculates the GHASH of data, with the key key, and input dst and
// data, and stores the resulting digest in dst. dst is both input and output.
@(enable_target_feature = TARGET_FEATURES)
ghash :: proc "contextless" (dst, key, data: []byte) #no_bounds_check {
	if len(dst) != _aes.GHASH_BLOCK_SIZE || len(key) != _aes.GHASH_BLOCK_SIZE {
		panic_contextless("aes/ghash: invalid dst or key size")
	}

	// GHASH numbers bits most-significant first. PMULL consumes polynomial
	// coefficients least-significant first, so reverse every 128-bit value.
	y0 := bits.reverse_bits(endian.unchecked_get_u64be(dst[0:]))
	y1 := bits.reverse_bits(endian.unchecked_get_u64be(dst[8:]))
	h0 := bits.reverse_bits(endian.unchecked_get_u64be(key[0:]))
	h1 := bits.reverse_bits(endian.unchecked_get_u64be(key[8:]))

	buf := data
	l := len(buf)
	for l > 0 {
		src: []byte = ---
		if l >= _aes.GHASH_BLOCK_SIZE {
			src = buf
			buf = buf[_aes.GHASH_BLOCK_SIZE:]
			l -= _aes.GHASH_BLOCK_SIZE
		} else {
			tmp: [_aes.GHASH_BLOCK_SIZE]byte
			copy(tmp[:], buf)
			src = tmp[:]
			l = 0
		}

		y0 ~= bits.reverse_bits(endian.unchecked_get_u64be(src[0:]))
		y1 ~= bits.reverse_bits(endian.unchecked_get_u64be(src[8:]))
		y0, y1 = gfmul(y0, y1, h0, h1)
	}

	endian.unchecked_put_u64be(dst[0:], bits.reverse_bits(y0))
	endian.unchecked_put_u64be(dst[8:], bits.reverse_bits(y1))
}
