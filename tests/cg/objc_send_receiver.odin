package cg

import "base:intrinsics"

@(objc_class="NSObject")
Object :: struct {using _: intrinsics.objc_object}

instance_retain_count :: proc(self: ^Object) -> int {
	return intrinsics.objc_send(int, self, "retainCount")
}

class_retain_count :: proc() -> int {
	return intrinsics.objc_send(int, Object, "retainCount")
}

main :: proc() {
	object: ^Object
	_ = instance_retain_count(object)
	_ = class_retain_count()
}
