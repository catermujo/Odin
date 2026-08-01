package cg_process_exec_zero_output

import "core:os"

main :: proc() {
	command := make([]string, 1, context.temp_allocator)
	command[0] = "/usr/bin/true"

	state, stdout, stderr, err := os.process_exec({command = command}, context.temp_allocator)
	assert(state.success)
	assert(len(stdout) == 0)
	assert(len(stderr) == 0)
	assert(err == nil)
}
