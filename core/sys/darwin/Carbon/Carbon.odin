package Carbon

import "core:c"

EventClass :: enum c.uint32_t {
	Keyboard = 0x6b657962,
}

EventKind :: enum c.uint32_t {
	HotKeyPressed = 5,
}

EventHotKeyModifier :: enum c.uint32_t {
	Command = 8,
	Shift   = 9,
	Option  = 11,
	Control = 12,
}

EventHotKeyModifiers :: bit_set[EventHotKeyModifier; c.uint32_t]

VirtualKeyCode :: enum c.uint32_t {
	ANSI_A = 0,
	ANSI_B = 11,
	ANSI_C = 8,
	ANSI_D = 2,
	ANSI_E = 14,
	ANSI_F = 3,
	ANSI_G = 5,
	ANSI_H = 4,
	ANSI_I = 34,
	ANSI_J = 38,
	ANSI_K = 40,
	ANSI_L = 37,
	ANSI_M = 46,
	ANSI_N = 45,
	ANSI_O = 31,
	ANSI_P = 35,
	ANSI_Q = 12,
	ANSI_R = 15,
	ANSI_S = 1,
	ANSI_T = 17,
	ANSI_U = 32,
	ANSI_V = 9,
	ANSI_W = 13,
	ANSI_X = 7,
	ANSI_Y = 16,
	ANSI_Z = 6,
}

EventTypeSpec :: struct {
	event_class: EventClass,
	event_kind:  EventKind,
}

EventHotKeyID :: struct {
	signature: c.uint32_t,
	id:        c.uint32_t,
}

EventTargetRef      :: distinct rawptr
EventHandlerRef     :: distinct rawptr
EventHotKeyRef      :: distinct rawptr
EventHandlerCallRef :: distinct rawptr
EventRef            :: distinct rawptr
OSStatus            :: distinct c.int32_t

EventHandlerUPP :: #type proc "c" (
	next_handler: EventHandlerCallRef,
	event: EventRef,
	user_data: rawptr,
) -> OSStatus

@(require) foreign import "system:Carbon.framework"

@(default_calling_convention = "c")
foreign Carbon {
	GetApplicationEventTarget :: proc() -> EventTargetRef ---
	InstallEventHandler :: proc(
		target: EventTargetRef,
		handler: EventHandlerUPP,
		event_type_count: c.uint32_t,
		event_types: ^EventTypeSpec,
		user_data: rawptr,
		out_handler: ^EventHandlerRef,
	) -> OSStatus ---
	RemoveEventHandler :: proc(handler: EventHandlerRef) -> OSStatus ---
	RegisterEventHotKey :: proc(
		key_code: VirtualKeyCode,
		modifiers: EventHotKeyModifiers,
		hotkey_id: EventHotKeyID,
		target: EventTargetRef,
		options: c.uint32_t,
		out_hotkey: ^EventHotKeyRef,
	) -> OSStatus ---
	UnregisterEventHotKey :: proc(hotkey: EventHotKeyRef) -> OSStatus ---
}
