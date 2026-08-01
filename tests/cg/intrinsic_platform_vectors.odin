package main

import "base:intrinsics"
import "base:builtin"

sqrt_f16 :: proc(value: f16) -> f16 {
	return intrinsics.sqrt(value)
}

cycle_counter :: proc() -> i64 {
	return intrinsics.read_cycle_counter()
}

when ODIN_ARCH == .arm64 {
	cycle_counter_frequency :: proc() -> i64 {
		return intrinsics.read_cycle_counter_frequency()
	}
}

map_info :: proc() -> rawptr {
	return rawptr(intrinsics.type_map_info(map[u32]u64))
}

map_cell_info :: proc() -> rawptr {
	return rawptr(intrinsics.type_map_cell_info(u8))
}

simd_indices_u8 :: proc() -> #simd[16]u8 {
	return intrinsics.simd_indices(#simd[16]u8)
}

simd_indices_f16 :: proc() -> #simd[4]f16 {
	return intrinsics.simd_indices(#simd[4]f16)
}

ptr_offset_multi :: proc(ptr: [^]u8) -> [^]u8 {
	return intrinsics.ptr_offset(ptr, 1)
}

ptr_sub_multi :: proc(a, b: [^]u8) -> int {
	return intrinsics.ptr_sub(a, b)
}

min_i128 :: proc() -> i128 {
	return min(i128)
}

max_i128 :: proc() -> i128 {
	return max(i128)
}

variadic_len :: proc(values: ..u32) -> uint {
	return builtin.len(values)
}

generic_variadic_len :: proc(_ : ^$T, values: ..T) -> uint {
	return builtin.len(values)
}

Distinct_Dynamic :: distinct [dynamic]i32

distinct_dynamic_sum :: proc(values: Distinct_Dynamic) -> int {
	total := 0
	for value, index in values {
		total += int(value) + index
	}
	return total
}

distinct_dynamic_with_init :: proc(values: Distinct_Dynamic) {
	for first := true; value in values {
		_ = value
		if first {
			first = false
		}
	}
}

Dynamic_Union :: union {
	Distinct_Dynamic,
	i32,
}

distinct_dynamic_union_sum :: proc(value: Dynamic_Union) -> int {
	#partial switch values in value {
	case Distinct_Dynamic:
		total := 0
		for item, index in values {
			total += int(item) + index
		}
		return total
	}
	return 0
}

interval_fallthrough :: proc(value: rune) -> int {
	result := 0
	switch value {
	case '+':
		result += 1
		fallthrough
	case '-':
		return result + 2
	case '0'..='9':
		return result + 3
	}
	return result
}

Bit_Field_Header :: bit_field u64 {
	previous: u64 | 32,
	is_free:  bool |  1,
	prev_ptr: u64 | 31,
}

bit_field_compound :: proc(previous, prev_ptr: u64) -> u64 {
	header := Bit_Field_Header{
		previous = previous,
		is_free = true,
		prev_ptr = prev_ptr,
	}
	return header.previous + header.prev_ptr + u64(header.is_free)
}

main :: proc() {
	_ = sqrt_f16(4)
	_ = cycle_counter()
	when ODIN_ARCH == .arm64 {
		_ = cycle_counter_frequency()
	}
	_ = map_info()
	_ = map_cell_info()
	_ = simd_indices_u8()
	_ = simd_indices_f16()
	_ = min_i128()
	_ = max_i128()
	_ = variadic_len(1, 2, 3)
	value: u32 = 1
	_ = generic_variadic_len(&value, value, value)
	_ = interval_fallthrough('+')
	_ = bit_field_compound(1, 2)
}
