package main

import "base:intrinsics"

@(export)
register :: proc "c" () -> rawptr {
	return intrinsics.objc_register_selector("retainCount")
}

main :: proc() {}
