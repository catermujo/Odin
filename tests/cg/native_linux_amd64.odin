package native_linux_amd64

import "base:intrinsics"

@(link_name="native_linux_amd64_data")
native_linux_amd64_data: u64 = 0x1122334455667788

@(link_name="native_linux_amd64_bss")
native_linux_amd64_bss: u64

@(link_name="native_linux_amd64_ptr")
native_linux_amd64_ptr := &native_linux_amd64_data

foreign {
	native_linux_amd64_foreign :: proc "c" (value: u64) -> u64 ---
	native_linux_amd64_control_foreign :: proc "c" (value: u64) -> u64 ---
	native_linux_amd64_narrow_foreign :: proc "c" (a: i8, b: u16, c: i32, d: u32, e: bool, f: i8, g: u16, h: u32) -> i16 ---
	native_linux_amd64_narrow_void :: proc "c" (value: i8) ---
	native_linux_amd64_float_foreign :: proc "c" (value: f64) -> f64 ---
	native_linux_amd64_float_void :: proc "c" (value: f64) ---
	native_linux_amd64_float_many :: proc "c" (a, b, c, d, e, f, g, h, i, j: f64) -> f64 ---
}

@(link_name="native_linux_amd64_syscall_zero")
@(export)
native_linux_amd64_syscall_zero :: proc "c" (number: uintptr) -> uintptr {
	return intrinsics.syscall(number)
}

@(link_name="native_linux_amd64_syscall_six")
@(export)
native_linux_amd64_syscall_six :: proc "c" (number, a, b, c, d, e, f: uintptr) -> uintptr {
	return intrinsics.syscall(number, a, b, c, d, e, f)
}

@(link_name="native_linux_amd64_syscall_expressions")
@(export)
native_linux_amd64_syscall_expressions :: proc "c" (number, a, b, c, d, e, f: uintptr) -> uintptr {
	return intrinsics.syscall(number ~ a, a + b, b ~ c, c + d, d ~ e, e + f, f ~ number)
}

@(link_name="native_linux_amd64_syscall_ordering")
@(export)
native_linux_amd64_syscall_ordering :: proc "c" (address: ^uintptr, number, before: uintptr) -> uintptr {
	address^ = before
	result := intrinsics.syscall(number, uintptr(address))
	observed := address^
	return result ~ observed
}

@(link_name="native_linux_amd64_syscall_control_ordering")
@(export)
native_linux_amd64_syscall_control_ordering :: proc "c" (address: ^uintptr, number, before: uintptr, choose: bool) -> uintptr {
	address^ = before
	result := intrinsics.syscall(number, uintptr(address))
	observed := address^
	if choose != false {
		return result ~ observed
	}
	return result + observed
}

@(link_name="native_linux_amd64_syscall_result_survival")
@(export)
native_linux_amd64_syscall_result_survival :: proc "c" (number, clobber: uintptr, condition: bool) -> uintptr {
	value := intrinsics.syscall(number)
	clobbered := uintptr(native_linux_amd64_foreign(u64(clobber)))
	if condition != false {
		return value ~ clobbered
	}
	return value + clobbered
}

@(link_name="native_linux_amd64_scalar")
@(export)
native_linux_amd64_scalar :: proc "c" (a, b: u64) -> u64 {
	x := a + b
	y := x * 3
	return y + 0x55
}

@(link_name="native_linux_amd64_pointer_float")
@(export)
native_linux_amd64_pointer_float :: proc "c" (destination: ^f64, value: f64) -> f64 {
	destination^ = value
	return destination^
}

@(link_name="native_linux_amd64_pointer_float_control")
@(export)
native_linux_amd64_pointer_float_control :: proc "c" (destination: ^f64, value: f64, choose: u64) -> f64 {
	if choose != 0 {
		destination^ = value
	} else {
		destination^ = value + 1
	}
	return destination^
}

@(link_name="native_linux_amd64_split_string")
@(export)
native_linux_amd64_split_string :: proc "c" (value: string) -> string { return value }

@(link_name="native_linux_amd64_split_string_forward")
@(export)
native_linux_amd64_split_string_forward :: proc "c" (value: string) -> string {
	return native_linux_amd64_split_string(value)
}

@(link_name="native_linux_amd64_split_string_control")
@(export)
native_linux_amd64_split_string_control :: proc "c" (first, second: string, choose: u64) -> string {
	if choose != 0 { return first }
	return second
}

@(link_name="native_linux_amd64_split_string_control_forward")
@(export)
native_linux_amd64_split_string_control_forward :: proc "c" (first, second: string, choose: u64) -> string {
	if choose != 0 { return native_linux_amd64_split_string_control(first, second, choose) }
	return native_linux_amd64_split_string_control(second, first, choose)
}

Native_Linux_AMD64_Large :: struct { first, second, third: u64 }

@(link_name="native_linux_amd64_sret")
@(export)
native_linux_amd64_sret :: proc "c" (first, second, third: u64) -> Native_Linux_AMD64_Large {
	return {first, second, third}
}

@(link_name="native_linux_amd64_sret_forward")
@(export)
native_linux_amd64_sret_forward :: proc "c" (first, second, third: u64) -> Native_Linux_AMD64_Large {
	return native_linux_amd64_sret(first, second, third)
}

@(link_name="native_linux_amd64_indirect_mutate")
@(export)
native_linux_amd64_indirect_mutate :: proc "c" (value: Native_Linux_AMD64_Large) -> Native_Linux_AMD64_Large {
	result := value
	result.first += 1
	return result
}

@(link_name="native_linux_amd64_indirect_forward")
@(export)
native_linux_amd64_indirect_forward :: proc "c" (value: Native_Linux_AMD64_Large) -> Native_Linux_AMD64_Large {
	return native_linux_amd64_indirect_mutate(value)
}

@(link_name="native_linux_amd64_sret_control")
@(export)
native_linux_amd64_sret_control :: proc "c" (first, second, third, choose: u64) -> Native_Linux_AMD64_Large {
	if choose != 0 { return {first, second, third} }
	return {third, second, first}
}

@(link_name="native_linux_amd64_sret_control_call")
@(export)
native_linux_amd64_sret_control_call :: proc "c" (first, second, third, choose: u64) -> Native_Linux_AMD64_Large {
	if choose != 0 { return native_linux_amd64_sret(first, second, third) }
	return native_linux_amd64_sret(third, second, first)
}

@(link_name="native_linux_amd64_six")
@(export)
native_linux_amd64_six :: proc "c" (a, b, c, d, e, f: u64) -> u64 {
	return (a + b) + (c + d) + (e + f)
}

@(link_name="native_linux_amd64_add")
@(export)
native_linux_amd64_add :: proc "c" (a, b: u64) -> u64 { return a + b }

@(link_name="native_linux_amd64_call_results")
@(export)
native_linux_amd64_call_results :: proc "c" (value: u64) -> u64 {
	a := native_linux_amd64_add(value, 1)
	b := native_linux_amd64_add(value, 2)
	return a + b
}

@(link_name="native_linux_amd64_bool_identity")
@(export)
native_linux_amd64_bool_identity :: proc "c" (value: bool) -> bool { return value }

@(link_name="native_linux_amd64_bool_direct")
@(export)
native_linux_amd64_bool_direct :: proc "c" (value: bool) -> u64 {
	if value { return 1 }
	return 0
}

@(link_name="native_linux_amd64_bool_local")
@(export)
native_linux_amd64_bool_local :: proc "c" (value: bool) -> u64 {
	local := value
	if local { return 1 }
	return 0
}

@(link_name="native_linux_amd64_bool_call_result")
@(export)
native_linux_amd64_bool_call_result :: proc "c" (value: bool) -> u64 {
	if native_linux_amd64_bool_identity(value) { return 1 }
	return 0
}

@(link_name="native_linux_amd64_bool_phi")
@(export)
native_linux_amd64_bool_phi :: proc "c" (value, choose: bool) -> u64 {
	condition := value
	if choose { condition = false } else { condition = true }
	if condition { return 1 }
	return 0
}

@(link_name="native_linux_amd64_bool_loop")
@(export)
native_linux_amd64_bool_loop :: proc "c" (value: bool) -> u64 {
	condition := value
	for condition { condition = false }
	return u64(condition)
}

@(link_name="native_linux_amd64_foreign_call")
@(export)
native_linux_amd64_foreign_call :: proc "c" (value: u64) -> u64 {
	return native_linux_amd64_foreign(value)
}

@(link_name="native_linux_amd64_eight")
@(export)
native_linux_amd64_eight :: proc "c" (a, b, c, d, e, f, g, h: u64) -> u64 { return a+b+c+d+e+f+g+h }

@(link_name="native_linux_amd64_call_eight")
@(export)
native_linux_amd64_call_eight :: proc "c" (value: u64) -> u64 {
	return native_linux_amd64_eight(value, 1, 2, 3, 4, 5, 6, 7)
}

@(link_name="native_linux_amd64_if_else")
@(export)
native_linux_amd64_if_else :: proc "c" (a, b: u64) -> u64 {
	if a < b {
		return b - a
	}
	return a - b
}

@(link_name="native_linux_amd64_signed_if")
@(export)
native_linux_amd64_signed_if :: proc "c" (a, b: i64) -> i64 {
	if a < b {
		return b
	}
	return a
}

@(link_name="native_linux_amd64_loop_sum")
@(export)
native_linux_amd64_loop_sum :: proc "c" (count: u64) -> u64 {
	result := u64(0)
	for index := u64(0); index < count; index += 1 {
		result += index
	}
	return result
}

@(link_name="native_linux_amd64_local_old_load")
@(export)
native_linux_amd64_local_old_load :: proc "c" (value: u64) -> u64 {
	current := value
	old := current
	current += 1
	if old != 0 {
		return value
	}
	return value
}

@(link_name="native_linux_amd64_pointer_phi")
@(export)
native_linux_amd64_pointer_phi :: proc "c" (left, right: rawptr, choose_left: u64) -> rawptr {
	if choose_left != 0 {
		return left
	}
	return right
}

@(link_name="native_linux_amd64_control_calls")
@(export)
native_linux_amd64_control_calls :: proc "c" (value: u64) -> u64 {
	if value != 0 {
		first := native_linux_amd64_control_foreign(value)
		second := native_linux_amd64_control_foreign(value + 1)
		return first + second
	}
	return native_linux_amd64_control_foreign(value)
}

@(link_name="native_linux_amd64_control_loop_calls")
@(export)
native_linux_amd64_control_loop_calls :: proc "c" (count: u64) -> u64 {
	result := u64(0)
	for index := u64(0); index < count; index += 1 {
		result += native_linux_amd64_control_foreign(index)
	}
	return result
}

@(link_name="native_linux_amd64_control_call_eight")
@(export)
native_linux_amd64_control_call_eight :: proc "c" (value: u64) -> u64 {
	if value != 0 {
		return native_linux_amd64_eight(value, 1, 2, 3, 4, 5, 6, 7)
	}
	return native_linux_amd64_eight(value, 7, 6, 5, 4, 3, 2, 1)
}

@(link_name="native_linux_amd64_narrow_math")
@(export)
native_linux_amd64_narrow_math :: proc "c" (a: i8, b: u8, c: i16, d: u16, e: i32, f: u32, flag: bool) -> i32 {
	if a < 0 {
		return i32(c + i16(b)) + e
	}
	return i32(d) + i32(f)
}

@(link_name="native_linux_amd64_narrow_unsigned_compare")
@(export)
native_linux_amd64_narrow_unsigned_compare :: proc "c" (left, right: u16) -> u16 {
	if left < right {
		return left
	}
	return right
}

@(link_name="native_linux_amd64_narrow_direct")
@(export)
native_linux_amd64_narrow_direct :: proc "c" (value: i8, count: u16) -> i16 {
	return i16(value) + i16(count)
}

@(link_name="native_linux_amd64_narrow_multiply")
@(export)
native_linux_amd64_narrow_multiply :: proc "c" (left: i8, right: i8) -> i8 {
	return left * right
}

@(link_name="native_linux_amd64_narrow_local")
@(export)
native_linux_amd64_narrow_local :: proc "c" (value: i16) -> i16 {
	local := value
	local += 1
	return local
}

@(link_name="native_linux_amd64_narrow_calls")
@(export)
native_linux_amd64_narrow_calls :: proc "c" (value: i8, count: u16, signed: i32, bits: u32, flag: bool) -> i16 {
	first := native_linux_amd64_narrow_direct(value, count)
	return native_linux_amd64_narrow_foreign(value, count, signed, bits, flag, value, count, bits) + first
}

@(link_name="native_linux_amd64_narrow_result_then_void")
@(export)
native_linux_amd64_narrow_result_then_void :: proc "c" (value: i8, count: u16) -> i16 {
	result := native_linux_amd64_narrow_direct(value, count)
	native_linux_amd64_narrow_void(value)
	return result
}

@(link_name="native_linux_amd64_narrow_control_call")
@(export)
native_linux_amd64_narrow_control_call :: proc "c" (value: i8, count: u16, flag: bool) -> i16 {
	if flag != false {
		return native_linux_amd64_narrow_direct(value, count)
	}
	return native_linux_amd64_narrow_direct(i8(0-i32(value)), count)
}

@(link_name="native_linux_amd64_narrow_sub_compare")
@(export)
native_linux_amd64_narrow_sub_compare :: proc "c" (value: i8) -> i8 {
	if i8(0)-value < i8(0) {
		return value
	}
	return i8(0)
}

@(link_name="native_linux_amd64_narrow_signext_compare")
@(export)
native_linux_amd64_narrow_signext_compare :: proc "c" (value: i8, limit: i32) -> i32 {
	if i32(value) < limit {
		return i32(value)
	}
	return limit
}

@(link_name="native_linux_amd64_u64_shift_div_rem")
@(export)
native_linux_amd64_u64_shift_div_rem :: proc "c" (lhs, rhs, count: u64) -> u64 {
	return (lhs << count) ~ (lhs >> count) ~ (lhs / rhs) ~ (lhs % rhs)
}

@(link_name="native_linux_amd64_i64_shift_div_rem")
@(export)
native_linux_amd64_i64_shift_div_rem :: proc "c" (lhs, rhs: i64, count: u64) -> i64 {
	return (lhs >> count) + (lhs / rhs) + (lhs % rhs)
}

@(link_name="native_linux_amd64_shift_call")
@(export)
native_linux_amd64_shift_call :: proc "c" (value, divisor, count: u64) -> u64 {
	return native_linux_amd64_add((value / divisor) << count, value % divisor)
}

@(link_name="native_linux_amd64_control_shift_div_call")
@(export)
native_linux_amd64_control_shift_div_call :: proc "c" (value, divisor, count: u64) -> u64 {
	if value < divisor {
		return native_linux_amd64_control_foreign((value << count) % divisor)
	}
	return native_linux_amd64_control_foreign((value / divisor) >> count)
}

@(link_name="native_linux_amd64_control_loop_shift_div_call")
@(export)
native_linux_amd64_control_loop_shift_div_call :: proc "c" (count, divisor, shift: u64) -> u64 {
	result := u64(0)
	for index := u64(0); index < count; index += 1 {
		value := index + 1
		result += native_linux_amd64_control_foreign((value / divisor) << shift) + value % divisor
	}
	return result
}

@(export)
native_linux_amd64_context_zero :: proc(value: u64) -> u64 {
	return value + 1
}

@(export)
native_linux_amd64_context_user_index :: proc(value: u64) -> u64 {
	return value + u64(context.user_index)
}

native_linux_amd64_context_no_args :: proc() -> u64 {
	return 1
}

@(export)
native_linux_amd64_context_no_args_call :: proc() -> u64 {
	return native_linux_amd64_context_no_args()
}

@(export)
native_linux_amd64_context_zero_call :: proc(value: u64) -> u64 {
	return native_linux_amd64_context_zero(value)
}

native_linux_amd64_context_five :: proc(a, b, c, d, e: u64) -> u64 {
	return a + b + c + d + e
}

@(export)
native_linux_amd64_context_five_call :: proc(value: u64) -> u64 {
	return native_linux_amd64_context_five(value, 1, 2, 3, 4)
}

native_linux_amd64_context_six :: proc(a, b, c, d, e, f: u64) -> u64 {
	return a + b + c + d + e + f
}

native_linux_amd64_context_six_float :: proc(a, b, c, d, e, f: u64, value: f64) -> f64 {
	return value + value
}

@(export)
native_linux_amd64_context_six_float_call :: proc(a, b, c, d, e, f: u64, value: f64) -> f64 {
	return native_linux_amd64_context_six_float(a, b, c, d, e, f, value)
}

@(export)
native_linux_amd64_context_six_call :: proc(value: u64) -> u64 {
	return native_linux_amd64_context_six(value, 1, 2, 3, 4, 5)
}

native_linux_amd64_context_mixed_callee :: proc(first: u64, value: f64, second: u64) -> f64 {
	return value + value
}

@(export)
native_linux_amd64_context_mixed_call :: proc(first: u64, value: f64, second: u64) -> f64 {
	return native_linux_amd64_context_mixed_callee(first, value, second)
}

@(export)
native_linux_amd64_context_control_call :: proc(value: u64) -> u64 {
	if value != 0 {
		return native_linux_amd64_context_zero(value)
	}
	return native_linux_amd64_context_zero(1)
}

@(export)
native_linux_amd64_context_loop_call :: proc(count: u64) -> u64 {
	result := u64(0)
	for index := u64(0); index < count; index += 1 {
		result += native_linux_amd64_context_zero(index)
	}
	return result
}

@(export)
native_linux_amd64_context_result_survives :: proc(value: u64) -> u64 {
	first := native_linux_amd64_context_zero(value)
	_ = native_linux_amd64_context_zero(value + 1)
	return first
}

@(link_name="native_linux_amd64_divzero_u64")
@(export)
native_linux_amd64_divzero_u64 :: proc "c" (divisor: u64) -> u64 { return 7 / divisor }

@(link_name="native_linux_amd64_modzero_u64")
@(export)
native_linux_amd64_modzero_u64 :: proc "c" (divisor: u64) -> u64 { return 7 % divisor }

@(link_name="native_linux_amd64_divzero_i64")
@(export)
native_linux_amd64_divzero_i64 :: proc "c" (divisor: i64) -> i64 { return 7 / divisor }

@(link_name="native_linux_amd64_modzero_i64")
@(export)
native_linux_amd64_modzero_i64 :: proc "c" (divisor: i64) -> i64 { return 7 % divisor }

@(link_name="native_linux_amd64_div_overflow_i64")
@(export)
native_linux_amd64_div_overflow_i64 :: proc "c" (divisor: i64) -> i64 { return i64(-9223372036854775808) / divisor }

@(link_name="native_linux_amd64_float_math")
@(export)
native_linux_amd64_float_math :: proc "c" (a, b: f32) -> f32 { return (a + b) * (a - b) / 2.0 }

@(link_name="native_linux_amd64_float64_calls")
@(export)
native_linux_amd64_float64_calls :: proc "c" (value: f64) -> f64 {
	result := native_linux_amd64_float_foreign(value)
	native_linux_amd64_float_void(value)
	return result
}

@(link_name="native_linux_amd64_float_mixed")
@(export)
native_linux_amd64_float_mixed :: proc "c" (first: u64, a, b: f64, second: u64) -> f64 {
	return a + b
}

@(link_name="native_linux_amd64_float_overflow")
@(export)
native_linux_amd64_float_overflow :: proc "c" (a, b, c, d, e, f, g, h, i, j: f64) -> f64 {
	return native_linux_amd64_float_many(a, b, c, d, e, f, g, h, i, j)
}

@(link_name="native_linux_amd64_float_add")
@(export)
native_linux_amd64_float_add :: proc "c" (a, b: f64) -> f64 { return a + b }

@(link_name="native_linux_amd64_control_float_branch")
@(export)
native_linux_amd64_control_float_branch :: proc "c" (value, limit: f64) -> f64 {
	if value < limit {
		result := native_linux_amd64_float_foreign(value)
		native_linux_amd64_float_void(limit)
		return result
	}
	return native_linux_amd64_float_add(value, limit)
}

@(link_name="native_linux_amd64_control_float_loop")
@(export)
native_linux_amd64_control_float_loop :: proc "c" (value, step: f32, count: u64) -> f32 {
	result := value
	for index := u64(0); index < count; index += 1 {
		result += step
	}
	return result
}

@(link_name="native_linux_amd64_control_float_mixed")
@(export)
native_linux_amd64_control_float_mixed :: proc "c" (first: u64, a: f64, second: u64, b: f32) -> f64 {
	if first < second {
		return native_linux_amd64_float_add(a, a)
	}
	return native_linux_amd64_float_foreign(a)
}

@(link_name="native_linux_amd64_control_float_value_compare")
@(export)
native_linux_amd64_control_float_value_compare :: proc "c" (a, b: f64) -> f64 {
	different := a != b
	if different == true {
		return a
	}
	return b
}

@(link_name="native_linux_amd64_control_float_overflow")
@(export)
native_linux_amd64_control_float_overflow :: proc "c" (a, b, c, d, e, f, g, h, i, j: f64) -> f64 {
	if a < b {
		return i + j
	}
	return native_linux_amd64_float_many(a, b, c, d, e, f, g, h, i, j)
}
