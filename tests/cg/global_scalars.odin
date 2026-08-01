package main

Global_Enum :: enum u16 {Zero, Value = 0x1234}

global_i64: i64 = -0x0102030405060708
global_u32: u32 = 0x10203040
global_i128: i128 = -1
global_f16: f16 = 1.5
global_f32be: f32be = 1.5
global_f64: f64 = 1.5
global_bool: b32 = true
global_enum: Global_Enum = .Value
global_nil: ^u8 = nil

main :: proc() {
	if global_nil != nil {
		return
	}
	if global_i64 + i64(global_u32) + i64(global_i128) + i64(global_f16) +
		i64(global_f32be) + i64(global_f64) + i64(global_bool) + i64(global_enum) == 0 {
		return
	}
}
