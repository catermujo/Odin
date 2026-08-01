package main

import "base:intrinsics"

relax :: proc() {
	intrinsics.cpu_relax()
}

main :: proc() {
	relax()
}
