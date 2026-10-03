package cg_file_info_iterator

import "core:os"

make_file_info :: proc() -> os.File_Info {
	return {name = "entry"}
}

next_file_info :: proc(state: ^int) -> (info: os.File_Info, index: int, ok: bool) {
	index = state^
	if index >= 2 {
		return
	}
	state^ += 1
	info = make_file_info()
	ok = true
	return
}

main :: proc() {
	state := 0
	count := 0
	for info, index in next_file_info(&state) {
		assert(info.name == "entry")
		assert(index == count)
		count += 1
	}
	assert(count == 2)
}
