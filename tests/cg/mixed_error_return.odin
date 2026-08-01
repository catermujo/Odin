package cg_mixed_error_return

import "core:os"

return_broken_pipe :: proc() -> (bool, os.Error) {
	return false, .Broken_Pipe
}

main :: proc() {
	ok, err := return_broken_pipe()
	assert(!ok)
	assert(err == .Broken_Pipe)
}
