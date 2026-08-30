package objc_Foundation

when !ODIN_PLATFORM_SUBTARGET_IOS {
	@(require) foreign import appkit "system:Cocoa.framework"

	@(default_calling_convention = "c")
	foreign appkit {
		@(link_name = "NSAccessibilityPostNotification")
		AccessibilityPostNotification :: proc "c" (element: ^Object, notification: ^String) ---
	}
}
