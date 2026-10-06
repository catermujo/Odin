package host

import "core:dynlib"
import "core:os"

main :: proc() {
	assert(len(os.args) == 2)
	data := make([]u8, 16)
	defer delete(data)

	library, ok := dynlib.load_library(os.args[1])
	assert(ok)
	defer dynlib.unload_library(library)

	run_worker := cast(proc "c" ())dynlib.symbol_address(library, "run_worker")
	assert(run_worker != nil)
	for _ in 0..<3 {
		run_worker()
	}
}
