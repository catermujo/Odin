package cg_mixed_aggregate_return

Entry :: struct {
	name: string,
	stamp: i128,
	mode: i32,
}

make_entry :: proc() -> (Entry, int, bool) {
	return {name = "entry", stamp = 0x123456789abcdef, mode = 7}, 42, true
}

main :: proc() {
	entry, index, ok := make_entry()
	assert(entry.name == "entry")
	assert(entry.stamp == 0x123456789abcdef)
	assert(entry.mode == 7)
	assert(index == 42)
	assert(ok)
}
