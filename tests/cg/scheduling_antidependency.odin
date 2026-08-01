package main

import "base:intrinsics"

load_store_alias :: proc(p: ^int) -> int {
	result := p^
	p^ = 99
	return result
}

conditional_store :: proc(p: ^int, write: bool) -> int {
	result := p^
	if write {
		p^ = 99
	}
	return result
}

branch_local_store :: proc(p: ^int, left: bool) -> int {
	result := 0
	if left {
		result = p^
		p^ = 41
	} else {
		result = p^
		p^ = 42
	}
	return result
}

clobber :: #force_no_inline proc(p: ^int) -> int {
	p^ = 23
	return 0
}

pick_first :: #force_no_inline proc(first, _: int) -> int {
	return first
}

call_clobber :: proc(p: ^int) -> int {
	return pick_first(p^, clobber(p))
}

atomic_clobber :: proc(p: ^u32) -> u32 {
	old := intrinsics.atomic_load_explicit(p, .Relaxed)
	_ = intrinsics.atomic_exchange_explicit(p, 37, .Relaxed)
	return old
}

unused_atomic_load :: proc(p: ^u32) {
	_ = intrinsics.atomic_load_explicit(p, .Relaxed)
}

bulk_zero :: proc(p: ^[16]u32) -> u32 {
	old := p[0]
	intrinsics.mem_zero(p, size_of(p^))
	return old
}

main :: proc() {
	alias := 7
	assert(load_store_alias(&alias) == 7)
	assert(alias == 99)

	conditional := 17
	assert(conditional_store(&conditional, false) == 17)
	assert(conditional == 17)
	assert(conditional_store(&conditional, true) == 17)
	assert(conditional == 99)

	branch := 31
	assert(branch_local_store(&branch, true) == 31)
	assert(branch == 41)
	branch = 32
	assert(branch_local_store(&branch, false) == 32)
	assert(branch == 42)

	call := 19
	assert(call_clobber(&call) == 19)
	assert(call == 23)

	atomic: u32 = 29
	assert(atomic_clobber(&atomic) == 29)
	assert(atomic == 37)
	unused_atomic_load(&atomic)

	bulk := [16]u32{53, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}
	assert(bulk_zero(&bulk) == 53)
	assert(bulk == [16]u32{})
}
