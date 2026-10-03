package main

import "core:sync"

guard_fallthrough :: proc(m: ^sync.Mutex) {
	if sync.mutex_guard(m) {
	}
}

guard_return :: proc(m: ^sync.Mutex) {
	if sync.mutex_guard(m) {
		return
	}
}

main :: proc() {
	m: sync.Mutex
	guard_fallthrough(&m)
	assert(sync.mutex_try_lock(&m))
	sync.mutex_unlock(&m)
	guard_return(&m)
	assert(sync.mutex_try_lock(&m))
	sync.mutex_unlock(&m)
}
