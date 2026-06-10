// Test: BitSet as cdecl proc arg/return
//
// BitSet is a built-in aggregate type in Odin. The fast-backend
// currently has no `case Type_BitSet:` in
// `fast_backend_type_is_supported_aggregate`, so procs that take
// or return a BitSet fall back to LLVM at the proc level. This
// test exercises the round-trip and prints the LLVM-emitted
// prologues for `mk_bits` and `echo` to document the current
// state.
//
// The cdecl callee here uses `transmute(Bits)u8(...)` rather than
// a BitSet compound literal `{0, 2, 4, 6}`, because the
// scalar-compound-literal emit path has no Type_BitSet case —
// that's a separate (and broader) follow-on: it would need to
// OR `(1<<i)` for each bit index rather than store at field
// offsets. Same for indexing, assignment, and the BitSet ABI
// paths in the call site / sret emit.
//
// BitSet layout matches its `underlying` integer: a small BitSet
// (8 bits with a u8 backing) is laid out as a single u8; a
// larger one (128 bits) is `[16]u8`. So treating it as an
// integer-shaped sequence in the aggregate paths should "just
// work" once the type is added to the supported set.
package test_bitset

import "core:fmt"

Bits :: bit_set[0..<8; u8]   // 1 byte, underlying = u8

mk_bits :: proc "c" () -> Bits {
	return transmute(Bits)u8(0x55) // 0101_0101 — bits 0, 2, 4, 6 set
}

echo :: proc "c" (b: Bits) -> int {
	x: u8 = (transmute(u8)b)
	n: int = 0
	if x & 0x01 != 0 { n += 0 }
	if x & 0x02 != 0 { n += 1 }
	if x & 0x04 != 0 { n += 2 }
	if x & 0x08 != 0 { n += 3 }
	if x & 0x10 != 0 { n += 4 }
	if x & 0x20 != 0 { n += 5 }
	if x & 0x40 != 0 { n += 6 }
	if x & 0x80 != 0 { n += 7 }
	return n
}

g_b:  Bits
g_n:  int

do_test :: proc() {
	g_b = mk_bits()
	g_n = echo(g_b)
}

main :: proc() {
	do_test()
	fmt.println("g_n =", g_n) // expect 0+2+4+6 = 12
	if g_n != 12 {
		fmt.println("FAIL")
	}
}
