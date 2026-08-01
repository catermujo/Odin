package native_runtime_containers

import "base:intrinsics"

foreign {
	native_runtime_pointer_sink :: proc "c" (value: ^u8) ---
	native_runtime_zero_sink    :: proc "c" (value: ^Zero) ---
	native_runtime_zero_pair    :: proc "c" (left, right: ^Zero) ---
}

Zero :: struct {}

Runtime_State :: struct {
	left:  i32,
	right: i32,
}

Runtime_Triple :: struct {
	first:  u8,
	second: u8,
	third:  u8,
}

@(export)
native_runtime_direct_bool :: proc "c" (value: i32) -> i32 {
	if value != 0 {
		return value
	}
	return 0
}

@(export)
native_runtime_expect_i8_truth :: proc "c" (value: u8) -> i32 {
	if intrinsics.expect(value != 0, false) {
		return 1
	}
	return 0
}

@(export)
native_runtime_expect_i64_truth :: proc "c" (value: uintptr) -> i32 {
	if intrinsics.expect(value & 0xff != 0, false) {
		return 1
	}
	return 0
}

native_runtime_truth_once :: proc "contextless" (counter: ^i32, value: uintptr) -> bool {
	counter^ += 1
	return value & 0xff != 0
}

@(export)
native_runtime_single_truth_evaluation :: proc "c" (counter: ^i32, value: uintptr) -> i32 {
	if native_runtime_truth_once(counter, value) {
		return counter^
	}
	return 0
}

@(export)
native_runtime_pointer_i64 :: proc "c" (base: [^]u8, index, stride: i64) -> u8 {
	return base[index*stride]
}

@(export)
native_runtime_pointer_i32 :: proc "c" (base: [^]u8, index: i32) -> u8 {
	return base[index]
}

@(export)
native_runtime_pointer_negative_i32 :: proc "c" (middle: [^]u8, distance: i32) -> u8 {
	return middle[-distance]
}

@(export)
native_runtime_pointer_scale_three :: proc "c" (base: [^]u8, index: i64) -> u8 {
	return base[index*3]
}

@(export)
native_runtime_pointer_pair :: proc "c" (base: [^]u8, left_index, left_stride, right_index, right_stride: i64) -> u16 {
	left := &base[left_index*left_stride]
	right := &base[right_index*right_stride]
	return u16(left^) + u16(right^)
}

@(export)
native_runtime_pointer_across_call :: proc "c" (base: [^]u8, index, stride: i64) -> u8 {
	value := &base[index*stride]
	native_runtime_pointer_sink(value)
	return value^
}

@(export)
native_runtime_zero_address :: proc "c" () {
	value: Zero
	native_runtime_zero_sink(&value)
}

@(export)
native_runtime_zero_pair_address :: proc "c" () {
	left, right: Zero
	native_runtime_zero_pair(&left, &right)
}

@(export)
native_runtime_zero_dynamic_stack :: proc "c" (size: int) {
	value: Zero
	data := intrinsics.alloca(size, 16)[:size]
	if size != 0 {
		data[0] = 1
	}
	native_runtime_zero_sink(&value)
}

@(export)
native_runtime_zero_map :: proc "c" (values: ^map[i32]Zero, key: i32) -> bool {
	values[key] = {}
	_, ok := values[key]
	return ok
}

@(export)
native_runtime_scalar_map :: proc "c" (values: ^map[i32]i64, key: i32, value: i64) -> i64 {
	values[key] = value
	return values[key]
}

@(export)
native_runtime_aggregate_map :: proc "c" (values: ^map[i32]Runtime_State, key: i32, value: Runtime_State) -> Runtime_State {
	values[key] = value
	return values[key]
}

@(export)
native_runtime_append_one :: proc(values: ^[dynamic]Runtime_Triple, value: Runtime_Triple) -> int {
	append(values, value)
	return len(values^)
}

@(export)
native_runtime_append_many :: proc(values: ^[dynamic]Runtime_Triple, source: []Runtime_Triple) -> int {
	append(values, ..source)
	return len(values^)
}

main :: proc() {}
