package native_variadic_abi

import "base:intrinsics"

foreign {
	native_variadic_sink :: proc "c" (value: int) ---
}

Small :: struct {
	first:  i32,
	second: i32,
}

Large_Result :: struct {
	first:  u64,
	second: u64,
	third:  u64,
}

native_variadic_count :: proc(values: ..i32) -> int {
	return len(values)
}

native_variadic_context_count :: proc(values: ..i32) -> int {
	return len(values) + int(context.user_index)
}

native_variadic_contextless_count :: proc "contextless" (values: ..i32) -> int {
	return len(values)
}

native_variadic_trailing :: proc(values: ..i32, marker := i32(0)) -> int {
	return len(values) + int(marker)
}

native_variadic_location :: proc(values: ..i32, loc := #caller_location) -> int {
	return len(values) + int(loc.line)
}

native_variadic_stack_callee :: proc(
	a0, a1, a2, a3, a4, a5, a6: u64,
	values: ..i32,
	marker := u64(0),
) -> u64 {
	return a0 + a1 + a2 + a3 + a4 + a5 + a6 + u64(len(values)) + marker + u64(context.user_index)
}

native_variadic_identity :: proc(values: ..i32) -> []i32 {
	return values
}

native_variadic_capture_any :: proc(values: ..any) -> int {
	return len(values)
}

@(export)
native_variadic_sret_callee :: proc(values: ..u64) -> Large_Result {
	first, last := u64(0), u64(0)
	if len(values) != 0 {
		first = values[0]
		last = values[len(values)-1]
	}
	return {first, last, u64(len(values))}
}

@(export)
native_variadic_sret_across_call :: proc() -> u64 {
	result := native_variadic_sret_callee(11, 22, 33)
	native_variadic_sink(int(result.second))
	return result.first + result.second + result.third
}

@(export)
native_variadic_empty :: proc() -> int {
	return native_variadic_count()
}

@(export)
native_variadic_one :: proc() -> int {
	return native_variadic_count(11)
}

@(export)
native_variadic_several :: proc() -> int {
	return native_variadic_count(11, 22, 33, 44)
}

@(export)
native_variadic_forward :: proc(values: []i32) -> int {
	return native_variadic_count(..values)
}

@(export)
native_variadic_forward_named :: proc(values: []i32) -> int {
	return native_variadic_count(values=values)
}

@(export)
native_variadic_default_location :: proc() -> int {
	return native_variadic_location(11, 22)
}

@(export)
native_variadic_explicit_location :: proc(values: []i32, loc := #caller_location) -> int {
	return native_variadic_location(..values, loc=loc)
}

@(export)
native_variadic_explicit_trailing :: proc() -> int {
	return native_variadic_trailing(11, 22, marker=33)
}

@(export)
native_variadic_context :: proc() -> int {
	return native_variadic_context_count(11, 22)
}

@(export)
native_variadic_call_contextless :: proc "c" () -> int {
	return native_variadic_contextless_count(11, 22)
}

@(export)
native_variadic_incoming_contextless :: proc "contextless" (values: ..i32) -> int {
	return len(values)
}

@(export)
native_variadic_outgoing_stack :: proc() -> u64 {
	return native_variadic_stack_callee(11, 22, 33, 44, 55, 66, 77, 88, 99, marker=111)
}

@(export)
native_variadic_incoming_stack :: proc(
	a0, a1, a2, a3, a4, a5, a6: u64,
	values: ..i32,
	marker := u64(0),
) -> u64 {
	return a0 + a1 + a2 + a3 + a4 + a5 + a6 + u64(len(values)) + marker + u64(context.user_index)
}

@(export)
native_variadic_dynamic_incoming_stack :: proc(
	size: int,
	a1, a2, a3, a4, a5, a6: u64,
	values: ..i32,
	marker := u64(0),
) -> u64 {
	data := intrinsics.alloca(size, 16)[:size]
	extra := u64(0)
	if size != 0 {
		data[0] = u8(marker)
		extra = u64(data[0])
	}
	return u64(size) + a1 + a2 + a3 + a4 + a5 + a6 + u64(len(values)) + marker + u64(context.user_index) + extra
}

@(export)
native_variadic_any_values :: proc() -> int {
	return native_variadic_capture_any(i32(11), "twenty-two", Small{33, 44})
}

@(export)
native_variadic_map_value_any :: proc(values: ^map[i32]Small, key: i32) -> int {
	return native_variadic_capture_any(values[key])
}

@(export)
native_variadic_result_across_call :: proc(values: []i32) -> int {
	result := native_variadic_identity(..values)
	native_variadic_sink(len(result))
	return len(result)
}

main :: proc() {}
