package cg_mixed_aggregate_iterator

Entry :: struct {
	name: string,
	stamp: i128,
	mode: i32,
}

next_entry :: proc(state: ^int) -> (Entry, int, bool) {
	if state^ >= 2 {
		return {}, 0, false
	}
	index := state^
	state^ += 1
	return {name = "entry", stamp = 0x123456789abcdef, mode = 7}, index, true
}

main :: proc() {
	state := 0
	count := 0
	for entry, index in next_entry(&state) {
		assert(entry.name == "entry")
		assert(entry.stamp == 0x123456789abcdef)
		assert(entry.mode == 7)
		assert(index == count)
		count += 1
	}
	assert(count == 2)
}
