package main

import "base:intrinsics"
import "base:runtime"

foreign {
	consume_block :: proc "c" (block: rawptr) ---
}

global_handler :: proc "c" (value: int) -> int {
	return value + 1
}

captured_handler :: proc "c" (value, delta: int) -> int {
	return value + delta
}

Big :: struct {
	a, b, c: int,
}

@(objc_class="NSObject")
Object :: struct {using _: intrinsics.objc_object}

aggregate_handler :: proc "c" (value: int, captured: Big) -> Big {
	return captured
}

void_handler :: proc "c" (captured: Big) {
	_ = captured
}

odin_handler :: proc(value: int) -> int {
	return value + 1
}

object_handler :: proc "c" (object: ^Object) {
	_ = object
}

block_handler :: proc "c" (block: ^runtime.Objc_Block(proc "c" (value: int) -> int)) {
	_ = block
}

global_block :: proc() {
	consume_block(intrinsics.objc_block(global_handler))
}

captured_block :: proc(value: int) {
	consume_block(intrinsics.objc_block(value, captured_handler))
}

aggregate_block :: proc(value: Big) {
	consume_block(intrinsics.objc_block(value, aggregate_handler))
	consume_block(intrinsics.objc_block(value, void_handler))
}

odin_block :: proc() {
	context = runtime.default_context()
	consume_block(intrinsics.objc_block(odin_handler))
}

object_block :: proc(object: ^Object) {
	consume_block(intrinsics.objc_block(object, object_handler))
}

block_capture_block :: proc() {
	block := intrinsics.objc_block(global_handler)
	consume_block(intrinsics.objc_block(block, block_handler))
}

main :: proc() {
	global_block()
	captured_block(2)
	aggregate_block({1, 2, 3})
	odin_block()
	object_block(nil)
	block_capture_block()
}
