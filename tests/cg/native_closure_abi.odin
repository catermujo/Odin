package native_closure_abi

import "base:intrinsics"

Small :: struct {
	left:  u64,
	right: u64,
}

Large :: struct {
	data: [4096]u64,
}

Closure_Box :: struct {
	call:  lambda(i64) -> i64,
	stamp: [3]u64,
}

apply_i64 :: proc(call: lambda(i64) -> i64, value: i64) -> i64 {
	return call(value)
}

apply_zero :: proc(call: lambda() -> i64) -> i64 {
	return call()
}

apply_small :: proc(call: lambda(u64) -> Small, value: u64) -> Small {
	return call(value)
}

apply_large :: proc(call: lambda(u64) -> Large, value: u64) -> Large {
	return call(value)
}

apply_gpr7 :: proc(
	call: lambda(u64, u64, u64, u64, u64, u64, u64) -> u64,
	a0, a1, a2, a3, a4, a5, a6: u64,
) -> u64 {
	return call(a0, a1, a2, a3, a4, a5, a6)
}

apply_gpr8 :: proc(
	call: lambda(u64, u64, u64, u64, u64, u64, u64, u64) -> u64,
	a0, a1, a2, a3, a4, a5, a6, a7: u64,
) -> u64 {
	return call(a0, a1, a2, a3, a4, a5, a6, a7)
}

apply_narrow :: proc(
	call: lambda(
		u64,
		u64,
		u64,
		u64,
		u64,
		u64,
		u64,
		u64,
		bool,
		i8,
		u8,
		i16,
		u16,
		i32,
		u32,
	) -> i64,
	a0, a1, a2, a3, a4, a5, a6, a7: u64,
	b: bool,
	s8: i8,
	u8_value: u8,
	s16: i16,
	u16_value: u16,
	s32: i32,
	u32_value: u32,
) -> i64 {
	return call(a0, a1, a2, a3, a4, a5, a6, a7, b, s8, u8_value, s16, u16_value, s32, u32_value)
}

apply_float_stack :: proc(
	call: lambda(f64, f64, f64, f64, f64, f64, f64, f64, f16, f32) -> u64,
	f0, f1, f2, f3, f4, f5, f6, f7: f64,
	half: f16,
	single: f32,
) -> u64 {
	return call(f0, f1, f2, f3, f4, f5, f6, f7, half, single)
}

apply_mixed_stack :: proc(
	call: lambda(
		u64,
		u64,
		u64,
		u64,
		u64,
		u64,
		u64,
		u64,
		f64,
		f64,
		f64,
		f64,
		f64,
		f64,
		f64,
		f64,
		bool,
		i8,
		u16,
		f16,
		f32,
	) -> u64,
	a0, a1, a2, a3, a4, a5, a6, a7: u64,
	f0, f1, f2, f3, f4, f5, f6, f7: f64,
	b: bool,
	s8: i8,
	u16_value: u16,
	half: f16,
	single: f32,
) -> u64 {
	return call(
		a0,
		a1,
		a2,
		a3,
		a4,
		a5,
		a6,
		a7,
		f0,
		f1,
		f2,
		f3,
		f4,
		f5,
		f6,
		f7,
		b,
		s8,
		u16_value,
		half,
		single,
	)
}

apply_dynamic :: proc(
	call: lambda(u64, u64, u64, u64, u64, u64, u64, int) -> u64,
	a0, a1, a2, a3, a4, a5, a6: u64,
	size: int,
) -> u64 {
	return call(a0, a1, a2, a3, a4, a5, a6, size)
}

@(export)
native_closure_zero_capture :: proc(value: i64) -> i64 {
	identity := lambda [](input: i64) -> i64 { return input }
	copy := identity
	return apply_i64(copy, value)
}

@(export)
native_closure_null_environment :: proc() -> i64 {
	constant := lambda []() -> i64 { return 0x51 }
	return apply_zero(constant)
}

@(export)
native_closure_by_value :: proc(value: i64) -> i64 {
	bias := i64(0x31)
	add := lambda [bias](input: i64) -> i64 { return input + bias }
	copy := add
	return apply_i64(copy, value)
}

@(export)
native_closure_by_reference :: proc(value: i64) -> i64 {
	counter := value
	increment := lambda [&counter]() -> i64 {
		counter += 3
		return counter
	}
	first := increment()
	second := increment()
	return first + second + counter
}

@(export)
native_closure_mixed_capture :: proc(value: u64) -> u64 {
	byte_value := u8(0x23)
	half_value := u16(0x4567)
	word_value := u32(0x89abcdef)
	local_value := value
	pointer_value := &local_value
	read := lambda [byte_value, half_value, word_value, pointer_value]() -> u64 {
		return u64(byte_value) + u64(half_value) + u64(word_value) + pointer_value^
	}
	return read()
}

@(export)
native_closure_passed_argument :: proc(value: i64) -> i64 {
	bias := i64(0x71)
	add := lambda [bias](input: i64) -> i64 { return input + bias }
	return apply_i64(add, value)
}

make_box :: proc(value: i64) -> Closure_Box {
	bias := value
	add := lambda [bias](input: i64) -> i64 { return input + bias }
	return Closure_Box {
		call = add,
		stamp = {0x11111111, 0x22222222, 0x33333333},
	}
}

@(export)
native_closure_sret_box :: proc(value: i64) -> Closure_Box {
	return make_box(value)
}

@(export)
native_closure_sret_box_call :: proc(value: i64) -> i64 {
	box := make_box(value)
	return box.call(i64(box.stamp[1]))
}

@(export)
native_closure_small_result :: proc(value: u64) -> Small {
	bias := u64(0x61)
	make := lambda [bias](input: u64) -> Small {
		return Small{input + bias, input ~ bias}
	}
	return apply_small(make, value)
}

@(export)
native_closure_large_result :: proc(value: u64) -> Large {
	bias := u64(0x73)
	make := lambda [bias](input: u64) -> Large {
		result: Large
		result.data[0] = input + bias
		result.data[2048] = input ~ bias
		result.data[4095] = input - bias
		return result
	}
	return apply_large(make, value)
}

@(export)
native_closure_environment_x7 :: proc(seed: u64) -> u64 {
	bias := seed
	sum := lambda [bias](a0, a1, a2, a3, a4, a5, a6: u64) -> u64 {
		return bias + a0 + a1 + a2 + a3 + a4 + a5 + a6 + u64(context.user_index)
	}
	return apply_gpr7(sum, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16)
}

@(export)
native_closure_environment_stack :: proc(seed: u64) -> u64 {
	bias := seed
	sum := lambda [bias](a0, a1, a2, a3, a4, a5, a6, a7: u64) -> u64 {
		return bias + a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + u64(context.user_index)
	}
	return apply_gpr8(sum, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27)
}

@(export)
native_closure_narrow_stack :: proc(seed: i64) -> i64 {
	bias := seed
	sum := lambda [bias](
		a0, a1, a2, a3, a4, a5, a6, a7: u64,
		b: bool,
		s8: i8,
		u8_value: u8,
		s16: i16,
		u16_value: u16,
		s32: i32,
		u32_value: u32,
	) -> i64 {
		result := bias + i64(a0+a1+a2+a3+a4+a5+a6+a7)
		if b {
			result += 0x101
		}
		return result + i64(s8) + i64(u8_value) + i64(s16) + i64(u16_value) + i64(s32) + i64(u32_value) +
		       i64(context.user_index)
	}
	return apply_narrow(
		sum,
		0x30,
		0x31,
		0x32,
		0x33,
		0x34,
		0x35,
		0x36,
		0x37,
		true,
		-0x12,
		0xe3,
		-0x2345,
		0xd678,
		-0x1234567,
		0x89abcdef,
	)
}

@(export)
native_closure_float_stack :: proc(seed: u64) -> u64 {
	bias := seed
	sum := lambda [bias](
		f0, f1, f2, f3, f4, f5, f6, f7: f64,
		half: f16,
		single: f32,
	) -> u64 {
		half_bits := transmute(u16)half
		single_bits := transmute(u32)single
		return bias + u64(f0+f1+f2+f3+f4+f5+f6+f7) + u64(half_bits) + u64(single_bits) +
		       u64(context.user_index)
	}
	return apply_float_stack(sum, 1, 2, 3, 4, 5, 6, 7, 8, f16(1.5), f32(2.25))
}

@(export)
native_closure_mixed_stack :: proc(seed: u64) -> u64 {
	bias := seed
	sum := lambda [bias](
		a0, a1, a2, a3, a4, a5, a6, a7: u64,
		f0, f1, f2, f3, f4, f5, f6, f7: f64,
		b: bool,
		s8: i8,
		u16_value: u16,
		half: f16,
		single: f32,
	) -> u64 {
		result := bias + a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7
		result += u64(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7)
		if b {
			result += 0x202
		}
		return result + u64(i64(s8)+0x80) + u64(u16_value) + u64(transmute(u16)half) +
		       u64(transmute(u32)single) + u64(context.user_index)
	}
	return apply_mixed_stack(
		sum,
		0x40,
		0x41,
		0x42,
		0x43,
		0x44,
		0x45,
		0x46,
		0x47,
		10,
		11,
		12,
		13,
		14,
		15,
		16,
		17,
		true,
		-0x35,
		0xcdef,
		f16(3.5),
		f32(4.75),
	)
}

@(export)
native_closure_dynamic_alloca :: proc(seed: u64, size: int) -> u64 {
	bias := seed
	sum := lambda [bias](a0, a1, a2, a3, a4, a5, a6: u64, byte_count: int) -> u64 {
		data := intrinsics.alloca(byte_count+1, 32)[:byte_count+1]
		data[0] = u8(bias)
		return bias + a0 + a1 + a2 + a3 + a4 + a5 + a6 + u64(data[0]) + u64(context.user_index)
	}
	return apply_dynamic(sum, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, size)
}

large_flag_first :: proc(call: lambda(u64) -> Large, value: u64) -> (bool, Large) {
	return true, call(value)
}

large_flag_last :: proc(call: lambda(u64) -> Large, value: u64) -> (Large, bool) {
	return call(value), false
}

small_result :: proc(value: u64) -> Small {
	return Small{value + 3, value + 5}
}

@(export)
native_closure_large_result_pressure :: proc(seed: u64) -> u64 {
	bias_a := seed + 0x81
	bias_b := seed + 0x93
	make_a := lambda [bias_a](input: u64) -> Large {
		result: Large
		result.data[0] = input + bias_a
		result.data[2048] = input ~ bias_a
		result.data[4095] = input - bias_a
		return result
	}
	make_b := lambda [bias_b](input: u64) -> Large {
		result: Large
		result.data[0] = input + bias_b
		result.data[2048] = input ~ bias_b
		result.data[4095] = input - bias_b
		return result
	}
	flag_a, large_a := large_flag_first(make_a, 0xa1)
	large_b, flag_b := large_flag_last(make_b, 0xb2)
	small := small_result(0xc3)
	result := large_a.data[0] + large_a.data[2048] + large_a.data[4095]
	result += large_b.data[0] + large_b.data[2048] + large_b.data[4095]
	result += small.left + small.right
	if flag_a {
		result += 0x303
	}
	if flag_b {
		result += 0x404
	}
	return result
}

make_cloned_counter :: proc(value: i64) -> lambda() -> i64 {
	counter := value
	increment := lambda [counter]() -> i64 {
		counter += 1
		return counter
	}
	return closure_clone(increment, context.allocator)
}

@(export)
native_closure_clone_free_local :: proc(value: i64) -> i64 {
	counter := value
	increment := lambda [counter]() -> i64 {
		counter += 2
		return counter
	}
	cloned := closure_clone(increment, context.allocator)
	result := cloned()
	closure_free(cloned, context.allocator)
	return result
}

@(export)
native_closure_clone_return_call_free :: proc(value: i64) -> i64 {
	cloned := make_cloned_counter(value)
	first := cloned()
	second := cloned()
	closure_free(cloned, context.allocator)
	return first + second
}

main :: proc() {}
