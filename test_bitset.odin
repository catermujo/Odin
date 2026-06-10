// Test: BitSet type support
//
// BitSet is a built-in Type_BitSet in Odin. Fast-backend has
// `case Type_BitSet:` in fast_backend_type_is_supported_aggregate,
// so procs that take/return a BitSet can be fast-emitted.
//
// BitSet layout matches its `underlying` integer: a small BitSet
// (e.g. 8 bits with a u8 backing) is laid out as a single u8.
//
// This is the simplest possible test: a cdecl proc that takes
// and returns a BitSet unchanged. If the proc is fast-emitted
// and the value round-trips, basic support works.
package test_bitset

import "base:intrinsics"

Bits :: bit_set[0..<8; u8]

identity :: proc "c" (b: Bits) -> Bits {
	// More complex body: do some work before returning,
	// to make sure the return path is correct (not just
	// "arg register == return register" coincidence).
	c := b
	c |= c
	return c
}

mk_set :: proc "c" () -> Bits {
	// Compound lit with bit indices — these are bits to set.
	return Bits{0, 2, 4, 6}
}

or_set :: proc "c" (a, b: Bits) -> Bits {
	return a | b
}

test_index :: proc "c" (b: Bits, i: int) -> bool {
	return i in b
}

g_in:  Bits
g_out: Bits

do_test :: proc() {
	g_in  = transmute(Bits)u8(0xA5) // bits 0, 2, 5, 7 set → 1010_0101
	g_out = identity(g_in)

	// Test compound lit (Bits{0, 2, 4, 6} = bits 0,2,4,6 set = 0x55)
	lit := mk_set()
	if (transmute(u8)lit) != 0x55 {
		intrinsics.trap()
	}

	// Test OR (0xA5 | 0x55 = 0xF5)
	or_val := or_set(transmute(Bits)u8(0xA5), transmute(Bits)u8(0x55))
	if (transmute(u8)or_val) != 0xF5 {
		intrinsics.trap()
	}

	// Test `in` (indexing): bits 0, 2, 5, 7 are set in 0xA5
	if !test_index(transmute(Bits)u8(0xA5), 0) { intrinsics.trap() }
	if !test_index(transmute(Bits)u8(0xA5), 2) { intrinsics.trap() }
	if !test_index(transmute(Bits)u8(0xA5), 5) { intrinsics.trap() }
	if !test_index(transmute(Bits)u8(0xA5), 7) { intrinsics.trap() }
	if  test_index(transmute(Bits)u8(0xA5), 1) { intrinsics.trap() }
	if  test_index(transmute(Bits)u8(0xA5), 3) { intrinsics.trap() }
}

main :: proc() {
	do_test()
	x: u8 = (transmute(u8)g_in)
	y: u8 = (transmute(u8)g_out)
	// Expected: 0xA5 (bits 0, 2, 5, 7 set)
	if x != 0xA5 || y != 0xA5 {
		intrinsics.trap()
	}
}
