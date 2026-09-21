package objc_Foundation

@(objc_class="NSImage")
Image :: struct { using _: Object }

@(objc_type=Image, objc_name="alloc", objc_is_class_method=true)
Image_alloc :: proc "c" () -> ^Image {
	return msgSend(^Image, Image, "alloc")
}

@(objc_type=Image, objc_name="initWithSize")
Image_initWithSize :: proc "c" (self: ^Image, size: Size) -> ^Image {
	return msgSend(^Image, self, "initWithSize:", size)
}

@(objc_type=Image, objc_name="addRepresentation")
Image_addRepresentation :: proc(self: ^Image, rep: ^ImageRep) {
	msgSend(nil, self, "addRepresentation:", rep)
}

@(objc_type = Image, objc_name = "imageWithSystemSymbolName", objc_is_class_method = true)
Image_imageWithSystemSymbolName :: proc "c" (name, description: ^String) -> ^Image {
	return msgSend(^Image, Image, "imageWithSystemSymbolName:accessibilityDescription:", name, description)
}

@(objc_type = Image, objc_name = "setTemplate")
Image_setTemplate :: proc "c" (self: ^Image, is_template: bool) {
	msgSend(nil, self, "setTemplate:", is_template)
}
