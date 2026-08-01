package main

import "base:intrinsics"

foreign import ObjC "system:objc"

@(objc_class="NSObject")
Base :: struct {using _: intrinsics.objc_object}

foreign ObjC {
	@(objc_type=Base, objc_selector="description")
	Base_description :: proc "c" (self: ^Base) -> ^Base ---

	@(objc_type=Base, objc_selector="alloc", objc_is_class_method=true)
	Base_alloc :: proc "c" () -> ^Base ---
}

@(objc_class="OdinCGSuper", objc_implement, objc_superclass=Base)
Object :: struct {using _: intrinsics.objc_object}

description_from_super :: proc(self: ^Object) -> ^Base {
	return Base_description(intrinsics.objc_super(self))
}

description_direct :: proc(self: ^Base) -> ^Base {
	return Base_description(self)
}

class_alloc :: proc() -> ^Base {
	return Base_alloc()
}

main :: proc() {
	object: ^Object
	_ = description_from_super(object)
	_ = description_direct(nil)
	_ = class_alloc()
}
