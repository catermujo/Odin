package main

import "base:intrinsics"

Ivar :: struct {
	value: i64,
}

@(objc_class="OdinCGIvar", objc_implement, objc_ivar=Ivar)
Object :: struct {using _: intrinsics.objc_object}

get_ivar :: proc(self: ^Object) -> ^Ivar {
	return intrinsics.objc_ivar_get(self)
}

main :: proc() {
	object: ^Object
	_ = get_ivar(object)
}
