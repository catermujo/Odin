package native_stack_alloc

import "base:intrinsics"

foreign {
	native_stack_alloc_sink :: proc "c" (value: ^u8) ---
	native_stack_alloc_sink10 :: proc "c" (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9: u64) ---
}

@(export)
native_stack_alloc_constant :: proc "c" (value: u8) -> u8 {
	data := intrinsics.alloca(16, 16)[:16]
	data[0] = value
	return data[0]
}

@(export)
native_stack_alloc_dynamic :: proc "c" (size: int, value: u8) -> u8 {
	data := intrinsics.alloca(size, 16)[:size]
	if size != 0 {
		data[0] = value
		return data[0]
	}
	return 0
}

@(export)
native_stack_alloc_offset :: proc "c" (size: int, value: u8) -> u8 {
	data := intrinsics.alloca(size+8, 32)[1:size+1]
	if size != 0 {
		data[size-1] = value
		return data[size-1]
	}
	return 0
}

@(export)
native_stack_alloc_branch :: proc "c" (size: int, pick_first: bool) -> rawptr {
	if pick_first {
		return intrinsics.alloca(size, 32)
	}
	return intrinsics.alloca(size+1, 32)
}

@(export)
native_stack_alloc_loop :: proc "c" (size: int) -> int {
	data := intrinsics.alloca(size, 16)[:size]
	total := 0
	for i := 0; i < size; i += 1 {
		data[i] = u8(i)
		total += int(data[i])
	}
	return total
}

@(export)
native_stack_alloc_loop_alloc :: proc "c" (size: int) -> int {
	total := 0
	for i := 0; i < 2; i += 1 {
		data := intrinsics.alloca(size, 16)[:size]
		if size != 0 {
			data[0] = u8(i)
			total += int(data[0])
		}
	}
	return total
}

@(export)
native_stack_alloc_fixed_call :: proc "c" (size: int, value: u64, pick: bool) -> u64 {
	local := value
	data := intrinsics.alloca(size+1, 32)
	data[0] = u8(value)
	native_stack_alloc_sink(data)
	if pick {
		return local + u64(data[0])
	}
	return local
}

@(export)
native_stack_alloc_ten_args :: proc "c" (size: int, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9: u64) -> u64 {
	local := a0
	data := intrinsics.alloca(size, 32)
	data[0] = u8(a9)
	native_stack_alloc_sink10(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9)
	return local + u64(data[0])
}

@(export)
native_stack_alloc_incoming_integer :: proc "c" (size: int, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9: u64) -> u64 {
	data := intrinsics.alloca(size, 32)[:size]
	extra := u64(0)
	if size != 0 {
		data[0] = u8(a8)
		extra = u64(data[0])
	}
	return a8 + a9 + extra
}

@(export)
native_stack_alloc_incoming_f64 :: proc "c" (size: int, a0, a1, a2, a3, a4, a5, a6, a7, a8: f64) -> f64 {
	data := intrinsics.alloca(size, 32)[:size]
	extra := f64(0)
	if size != 0 {
		data[0] = u8(a8)
		extra = f64(data[0])
	}
	return a8 + extra
}

@(export)
native_stack_alloc_incoming_aggregate :: proc "c" (size: int, a0, a1, a2, a3, a4, a5, a6: u64, value: [3]u64) -> u64 {
	data := intrinsics.alloca(size, 32)[:size]
	extra := u64(0)
	if size != 0 {
		data[0] = u8(value[2])
		extra = u64(data[0])
	}
	return value[2] + extra
}

@(export)
native_stack_alloc_zero :: proc "c" () -> rawptr {
	return intrinsics.alloca(0, 16)
}

main :: proc() {}
