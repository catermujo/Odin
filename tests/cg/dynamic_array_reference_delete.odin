package main

import "core:mem"

Field :: struct {
	name:  string,
	value: i64,
}

Entry :: struct {
	kind:        u8,
	width:       u8,
	signed:      bool,
	elem:        u32,
	index:       u32,
	count:       int,
	base:        u32,
	fields:      [dynamic]Field,
	enum_fields: [dynamic]Field,
	id:          typeid,
}

Error :: struct {
	kind:      u8,
	offset:    int,
	path:      string,
	message:   string,
	path_owned: bool,
	allocator: mem.Allocator,
}

delete_entry :: proc(entry: ^Entry) {
	delete(entry.fields)
	delete(entry.enum_fields)
}

dispose :: proc(entries: [dynamic]Entry) {
	for &entry in entries do delete_entry(&entry)
	delete(entries)
}

make_entries :: proc() -> (entries: [dynamic]Entry, error: Error, ok: bool) {
	entries_error: mem.Allocator_Error
	entries, entries_error = make([dynamic]Entry, 0, 8)
	if entries_error != nil do return nil, {}, false
	defer if !ok do dispose(entries)
	for index in 0 ..< 8 {
		_, append_error := append(&entries, Entry{kind = u8(index), count = index})
		if append_error != nil do return nil, {}, false
		entry := &entries[len(entries) - 1]
		for value in 0 ..< index + 1 {
			_, fields_error := append(&entry.fields, Field{name = "field", value = i64(value)})
			_, enum_fields_error := append(&entry.enum_fields, Field{name = "enum", value = i64(value)})
			if fields_error != nil || enum_fields_error != nil do return nil, {}, false
		}
	}
	return entries, {}, true
}

main :: proc() {
	backing := context.allocator
	tracker: mem.Tracking_Allocator
	mem.tracking_allocator_init(&tracker, backing)
	tracker.bad_free_callback = mem.tracking_allocator_bad_free_callback_add_to_array
	context.allocator = mem.tracking_allocator(&tracker)

	entries, _, ok := make_entries()
	assert(ok)
	dispose(entries)

	context.allocator = backing
	assert(len(tracker.bad_free_array) == 0)
	mem.tracking_allocator_destroy(&tracker)
}
