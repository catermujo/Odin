package objc_Foundation

@(objc_class = "NSStatusBar")
StatusBar :: struct {using _: Object}

@(objc_class = "NSStatusItem")
StatusItem :: struct {using _: Object}

@(objc_class = "NSStatusBarButton")
StatusBarButton :: struct {using _: Object}

VariableStatusItemLength :: Float(-1)
SquareStatusItemLength   :: Float(-2)

@(objc_type = StatusBar, objc_name = "systemStatusBar", objc_is_class_method = true)
StatusBar_systemStatusBar :: proc "c" () -> ^StatusBar {
	return msgSend(^StatusBar, StatusBar, "systemStatusBar")
}

@(objc_type = StatusBar, objc_name = "statusItemWithLength")
StatusBar_statusItemWithLength :: proc "c" (self: ^StatusBar, length: Float) -> ^StatusItem {
	return msgSend(^StatusItem, self, "statusItemWithLength:", length)
}

@(objc_type = StatusBar, objc_name = "removeStatusItem")
StatusBar_removeStatusItem :: proc "c" (self: ^StatusBar, item: ^StatusItem) {
	msgSend(nil, self, "removeStatusItem:", item)
}

@(objc_type = StatusItem, objc_name = "button")
StatusItem_button :: proc "c" (self: ^StatusItem) -> ^StatusBarButton {
	return msgSend(^StatusBarButton, self, "button")
}

@(objc_type = StatusItem, objc_name = "setMenu")
StatusItem_setMenu :: proc "c" (self: ^StatusItem, menu: ^Menu) {
	msgSend(nil, self, "setMenu:", menu)
}

@(objc_type = StatusBarButton, objc_name = "setImage")
StatusBarButton_setImage :: proc "c" (self: ^StatusBarButton, image: ^Image) {
	msgSend(nil, self, "setImage:", image)
}

@(objc_type = StatusBarButton, objc_name = "setTitle")
StatusBarButton_setTitle :: proc "c" (self: ^StatusBarButton, title: ^String) {
	msgSend(nil, self, "setTitle:", title)
}

@(objc_type = StatusBarButton, objc_name = "setToolTip")
StatusBarButton_setToolTip :: proc "c" (self: ^StatusBarButton, tool_tip: ^String) {
	msgSend(nil, self, "setToolTip:", tool_tip)
}
