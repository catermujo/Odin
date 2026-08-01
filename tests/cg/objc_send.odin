package main

import Foundation "core:sys/darwin/Foundation"

drain :: proc(pool: ^Foundation.AutoreleasePool) {
	Foundation.AutoreleasePool_drain(pool)
}

main :: proc() {
	pool: ^Foundation.AutoreleasePool
	drain(pool)
}
