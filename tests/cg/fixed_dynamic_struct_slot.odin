package main

import "core:mem"

Handle :: struct {
	idx: u32,
	gen: u32,
}

Table :: struct {
	cols, rows: int,
	count:      int,
	slots: [dynamic; 64]Handle,
}

skip :: proc(handle: Handle) -> bool {
	return handle.idx == 0
}

table_index :: #force_inline proc(table: Table, key: int) -> (idx: int, ok: bool) {
	if key < 0 || key >= table.cols * table.rows do return
	return key, true
}

table_get :: #force_inline proc(table: ^Table, key: int) -> Maybe(Handle) {
	idx, ok := table_index(table^, key)
	if !ok do return nil
	handle := table.slots[idx]
	if skip(handle) do return nil
	return handle
}

table_get_ref :: proc(table: ^Table, key: int) -> Maybe(^Handle) {
	idx, ok := table_index(table^, key)
	if !ok do return nil
	handle := table.slots[idx]
	if skip(handle) do return nil
	return &table.slots[idx]
}

main :: proc() {
	table: Table
	table.cols = 4
	table.rows = 4
	assert(resize(&table.slots, 16))
	mem.zero_slice(table.slots[:])
	table.slots[3] = {idx = 7, gen = 2}
	assert(!skip(table.slots[3]))
	assert(table.slots[3].idx == 7)
	assert(table.slots[3].gen == 2)
	handle, ok := table_get(&table, 3).?
	assert(ok && handle.idx == 7 && handle.gen == 2)
	handle_ref, ref_ok := table_get_ref(&table, 3).?
	assert(ref_ok)
	assert(handle_ref != nil)
	assert(handle_ref.idx == 7)
	assert(handle_ref.gen == 2)
	missing_ref, missing_ok := table_get_ref(&table, 2).?
	assert(!missing_ok)
	assert(missing_ref == nil)
}
