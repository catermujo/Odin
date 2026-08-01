package native_frame_addressing

import "base:intrinsics"

foreign {
	native_frame_direct_sink :: proc "c" (value: ^u64) ---
}

Callback :: proc "c" (value: ^u64)

Big :: [6000]u8
Aligned :: struct #align(32) {
	value: [4]u64,
}

@(export)
native_frame_local_address :: proc "c" (value: u64) -> u64 {
	local := value
	p := &local
	p^ += 1
	return p^
}

@(export)
native_frame_local_offset :: proc "c" (value: u64) -> u64 {
	local := [8]u64{0, 0, 0, 0, 0, 0, 0, value}
	p := &local[7]
	return p^
}

@(export)
native_frame_aggregate_address :: proc "c" (value: u64) -> u64 {
	aggregate := Aggregate{}
	aggregate.values[3] = value
	p := &aggregate
	return p^.values[3]
}

@(export)
native_frame_nested_offset :: proc "c" (value: u64) -> u64 {
	local := [4]Pair{}
	local[3].value = value
	return local[3].value
}

@(export)
native_frame_direct_address_call :: proc "c" (value: u64) {
	local := value
	native_frame_direct_sink(&local)
}

@(export)
native_frame_indirect_address_call :: proc "c" (callback: Callback, value: u64) {
	local := value
	callback(&local)
}

@(export)
native_frame_store_address :: proc "c" (value: u64) -> u64 {
	local := value
	address: ^u64 = nil
	address = &local
	return address^
}

@(export)
native_frame_large_offset :: proc "c" (value: u8) -> u8 {
	local: Big
	local[5999] = value
	return local[5999]
}

@(export)
native_frame_aligned32 :: proc "c" (value: u64) -> u64 {
	local: Aligned
	local.value[3] = value
	return local.value[3]
}

@(export)
native_frame_incoming_stack_args :: proc "c" (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9: u64) -> u64 {
	local: Aligned
	local.value[0] = a8
	local.value[1] = a9
	return local.value[0] + local.value[1]
}

@(export)
native_frame_incoming_stack_f64 :: proc "c" (a0, a1, a2, a3, a4, a5, a6, a7, a8: f64) -> f64 {
	local: Aligned
	local.value[0] = u64(a8)
	return f64(local.value[0]) + a8
}

@(export)
native_frame_incoming_stack_aggregate :: proc "c" (a0, a1, a2, a3, a4, a5, a6, a7: u64, value: [3]u64) -> u64 {
	local: Aligned
	local.value[0] = value[2]
	return local.value[0]
}

@(export)
native_frame_address_call_stable :: proc "c" (value: u64) -> u64 {
	local := value
	p := &local
	native_frame_direct_sink(p)
	return p^
}

@(export)
native_frame_global_address :: proc "c" (value: u64) -> u64 {
	native_frame_global = value
	return native_frame_global
}

@(export)
native_frame_pointer_offset :: proc "c" (base: ^[8]u64, index: i64) -> u64 {
	return base[index]
}

@(export)
native_frame_pointer_store :: proc "c" (base: ^[8]u64, index: u32, value: u64) {
	base[index] = value
}

@(export)
native_frame_merge_address :: proc "c" (left, right: u64, pick: bool) -> u64 {
	a := left
	b := right
	p := &a if pick else &b
	return p^
}

@(export)
native_frame_loop_call :: proc "c" (value: u64, count: u32) -> u64 {
	local := value
	remaining := count
	for remaining != 0 {
		native_frame_direct_sink(&local)
		local += 1
		remaining -= 1
	}
	return local
}

@(export)
native_frame_volatile_local :: proc "c" (value: u64) -> u64 {
	local := value
	intrinsics.volatile_store(&local, value)
	return intrinsics.volatile_load(&local)
}

Pair :: struct {
	value: u64,
}

Aggregate :: struct {
	values: [4]u64,
}

@(export)
native_frame_global: u64

main :: proc() {}
