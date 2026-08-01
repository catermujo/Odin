package main

Global_Record :: struct {
	id: i64,
	count: u32,
	enabled: b32,
	weight: f32,
	label: string,
	c_label: cstring,
}

global_values: [3]u16 = {0x1122, 0x3344, 0x5566}
global_grid: [2][2]u32 = {{1, 2}, {3, 4}}
global_record: Global_Record = {
	id = -0x0102030405060708,
	count = 0x10203040,
	enabled = true,
	weight = 1.5,
	label = "CG aggregate",
	c_label = "CG cstring",
}
main :: proc() {
	if global_values[0] != 0x1122 || global_values[2] != 0x5566 ||
		global_grid[1][0] != 3 || len(global_record.label) != 12 ||
		global_record.c_label == nil {
		return
	}
}
