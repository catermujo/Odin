package cg

import "base:intrinsics"

memory_ops :: proc(read, write, zero: ^u32, size: int) -> u32 {
	intrinsics.volatile_store(write, 7)
	_ = intrinsics.volatile_load(write)
	intrinsics.volatile_store(write, 8)
	non_temporal_value := intrinsics.non_temporal_load(read)
	intrinsics.non_temporal_store(write, non_temporal_value)
	intrinsics.mem_zero_volatile(zero, size)
	intrinsics.atomic_thread_fence(.Seq_Cst)
	intrinsics.atomic_signal_fence(.Acquire)
	return non_temporal_value
}

@(export)
memory_width_ops :: proc(narrow: ^i8, wide: ^u64, real: ^f64, narrow_value: i8, wide_value: u64, real_value: f64) -> f64 {
	intrinsics.volatile_store(narrow, narrow_value)
	_ = intrinsics.volatile_load(narrow)
	intrinsics.volatile_store(wide, wide_value)
	wide_result := intrinsics.non_temporal_load(wide)
	intrinsics.non_temporal_store(wide, wide_result)
	intrinsics.volatile_store(real, real_value)
	return intrinsics.volatile_load(real)
}

main :: proc() {
	read, write, zero := u32(0), u32(0), u32(0)
	_ = memory_ops(&read, &write, &zero, size_of(u32))
}
