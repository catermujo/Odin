package library

import "base:runtime"
import "core:thread"

@(export)
run_worker :: proc "c" () {
	context = runtime.default_context()
	worker := thread.create_and_start(proc() {
		data := make([]u8, 64)
		data[0] = 42
		delete(data)
	})
	thread.join(worker)
	thread.destroy(worker)
}
