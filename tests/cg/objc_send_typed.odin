package main

import Foundation "core:sys/darwin/Foundation"

alloc_pool :: proc() -> ^Foundation.AutoreleasePool {
	return Foundation.AutoreleasePool_alloc()
}

responds :: proc(object: ^Foundation.Object, selector: Foundation.SEL) -> Foundation.BOOL {
	return Foundation.respondsToSelector(object, selector)
}

main :: proc() {
	pool := alloc_pool()
	_ = responds(pool, nil)
}
