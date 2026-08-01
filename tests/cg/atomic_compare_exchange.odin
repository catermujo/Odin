package main

import "base:intrinsics"

weak_compare_exchange :: proc(value: ^u32, old, new: u32) -> (u32, bool) {
	actual, ok := intrinsics.atomic_compare_exchange_weak_explicit(value, old, new, .Acquire, .Relaxed)
	return actual, ok
}

strong_compare_exchange :: proc(value: ^u32, old, new: u32) -> (u32, bool) {
	actual, ok := intrinsics.atomic_compare_exchange_strong(value, old, new)
	return actual, ok
}

weak_compare_exchange_implicit :: proc(value: ^u32, old, new: u32) -> (u32, bool) {
	actual, ok := intrinsics.atomic_compare_exchange_weak(value, old, new)
	return actual, ok
}

strong_compare_exchange_explicit :: proc(value: ^u32, old, new: u32) -> (u32, bool) {
	actual, ok := intrinsics.atomic_compare_exchange_strong_explicit(value, old, new, .Seq_Cst, .Acquire)
	return actual, ok
}

assignment_compare_exchange :: proc(value: ^u32, old, new: u32) -> (u32, bool) {
	actual: u32; ok: bool
	actual, ok = intrinsics.atomic_compare_exchange_strong_explicit(value, old, new, .Seq_Cst, .Acquire)
	return actual, ok
}

main :: proc() {
	value := u32(1)
	_, _ = weak_compare_exchange(&value, 1, 2)
	_, _ = strong_compare_exchange(&value, 2, 3)
	_, _ = weak_compare_exchange_implicit(&value, 3, 4)
	_, _ = strong_compare_exchange_explicit(&value, 4, 5)
	_, _ = assignment_compare_exchange(&value, 5, 6)
}
