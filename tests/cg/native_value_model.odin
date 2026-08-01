package native_value_model

import "base:intrinsics"

foreign {
	native_m13_sink :: proc "c" (lhs, rhs: u64) ---
	native_m13_sink_ptr :: proc "c" (address: ^u64) ---
}

@(export)
native_m13_global: u64

@(export)
native_m13_select_eq :: proc "c" (lhs, rhs: u32) -> u32 {
	return 11 if lhs == rhs else 22
}

@(export)
native_m13_select_ne :: proc "c" (lhs, rhs: u32) -> u32 {
	return 11 if lhs != rhs else 22
}

@(export)
native_m13_select_ult :: proc "c" (lhs, rhs: u32) -> u32 {
	return 11 if lhs < rhs else 22
}

@(export)
native_m13_select_ule :: proc "c" (lhs, rhs: u32) -> u32 {
	return 11 if lhs <= rhs else 22
}

@(export)
native_m13_select_ilt :: proc "c" (lhs, rhs: i32) -> i64 {
	return -11 if lhs < rhs else 22
}

@(export)
native_m13_select_ile :: proc "c" (lhs, rhs: i32) -> i64 {
	return -11 if lhs <= rhs else 22
}

@(export)
native_m13_select_node_ult :: proc "c" (lhs, rhs: u32) -> u32 {
	return min(lhs, rhs)
}

@(export)
native_m13_select_node_ilt :: proc "c" (lhs, rhs: i32) -> i32 {
	return max(lhs, rhs)
}

@(export)
native_m13_truncate :: proc "c" (value: u64) -> u32 {
	return u32(value)
}

@(export)
native_m13_sign_ext :: proc "c" (value: i32) -> i64 {
	return i64(value)
}

@(export)
native_m13_zero_ext :: proc "c" (value: u32) -> u64 {
	return u64(value)
}

@(export)
native_m13_count_ones_u32 :: proc "c" (value: u32) -> u32 {
	return intrinsics.count_ones(value)
}

@(export)
native_m13_count_ones_u64 :: proc "c" (value: u64) -> u64 {
	return intrinsics.count_ones(value)
}

@(export)
native_m13_local_values :: proc "c" (value: i32) -> i64 {
	local := i64(value)
	local = i64(intrinsics.count_ones(u32(value)))
	return local
}

@(export)
native_m13_global_store :: proc "c" (value: u64) {
	native_m13_global = intrinsics.count_ones(value)
}

@(export)
native_m13_call_values :: proc "c" (lhs, rhs: u32) {
	native_m13_sink(u64(min(lhs, rhs)), u64(intrinsics.count_ones(lhs)))
}

@(export)
native_m13_pointer_bits :: proc "c" (address: ^u64) {
	native_m13_sink_ptr((^u64)(uintptr(address)))
}

@(export)
native_m13_pointer_bits_value :: proc "c" (address: ^u64) -> u64 {
	return u64(uintptr(address))
}

@(export)
native_m13_loop_values :: proc "c" (value: u64, limit: u32) -> u64 {
	total := value
	i := limit
	for i != 0 {
		count := intrinsics.count_ones(u32(total))
		total = total + u64(count if (i & 1) == 0 else count + 1)
		if i == 2 {
			break
		}
		i -= 1
	}
	return total
}

main :: proc() {}
