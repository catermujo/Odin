package main

import "base:intrinsics"

@(export)
register :: proc "c" () -> rawptr {
	return intrinsics.objc_register_class("OdinCGTestClass")
}

main :: proc() {}
