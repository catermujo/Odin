package cg

import "base:intrinsics"

overflow_u :: proc() -> (u64, bool) {
	return intrinsics.overflow_mul(u64(1)<<63, u64(2))
}

overflow_s :: proc() -> (i64, bool) {
	return intrinsics.overflow_mul(i64(1)<<62, i64(4))
}

overflow_add :: proc() -> (u64, bool) {
	return intrinsics.overflow_add(~u64(0), u64(1))
}

overflow_sub :: proc() -> (u64, bool) {
	return intrinsics.overflow_sub(u64(0), u64(1))
}

main :: proc() {
	product_u, overflow_u := overflow_u()
	assert(product_u == 0)
	assert(overflow_u)

	product_s, overflow_s := overflow_s()
	assert(product_s == 0)
	assert(overflow_s)

	product_add, overflow_add := overflow_add()
	assert(product_add == 0)
	assert(overflow_add)

	product_sub, overflow_sub := overflow_sub()
	assert(product_sub == ~u64(0))
	assert(overflow_sub)
}
