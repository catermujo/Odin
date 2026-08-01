package main

import "closure_helper"
import "core:testing"

@(test)
separate_module_procedure_closure_test :: proc(t: ^testing.T) {
	_ = t

	assert(closure_helper.direct(41) == 42)

	ctx := closure_helper.Callback_Context{callback = closure_helper.callback_only}
	assert(ctx.callback(40) == 42)

	assert(closure_helper.global_callback(39) == 42)

	assert(closure_helper.callback_generated(40) == 42)
}
