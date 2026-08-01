package main

import "core:testing"
import "immutable_helper"

@(test)
cross_module_immutable_globals_test :: proc(t: ^testing.T) {
	_ = t
	assert(immutable_helper.check("M6E shared immutable literal"))
	assert(immutable_helper.utf16_checksum() == 0x1B767)
}
