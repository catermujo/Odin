package native_tls_arm64

Native_TLS_Vector :: #simd[4]f32

Native_TLS_Block :: struct {
	header:  u32,
	payload: [3]u64,
	value:   f64,
}

Native_TLS_Result :: struct {
	first:  u64,
	second: u64,
	third:  u64,
	fourth: u64,
}

Native_TLS_Call :: proc "c" (address: ^u64, value: u64) -> u64

foreign {
	native_tls_address_sink :: proc "c" (address: rawptr) -> uintptr ---
}

@(thread_local)
native_tls_file_default: u64

@(thread_local="globaldynamic")
native_tls_file_globaldynamic: u64

@(thread_local="localdynamic")
native_tls_file_localdynamic: u64

@(thread_local="initialexec")
native_tls_file_initialexec: u64

@(thread_local="localexec")
native_tls_file_localexec: u64

@(thread_local)
native_tls_file_f64: f64

@(thread_local)
native_tls_file_vector: Native_TLS_Vector

@(thread_local)
native_tls_file_block: Native_TLS_Block

native_tls_ordinary_file: u64

native_tls_direct_callee :: proc "c" (address: ^u64, value: u64) -> u64 {
	return address^ + value
}

native_tls_indirect_callee :: proc "c" (address: ^u64, value: u64) -> u64 {
	return address^ ~ value
}

@(export)
native_tls_file_read :: proc "c" () -> u64 {
	return native_tls_file_default
}

@(export)
native_tls_file_write :: proc "c" (value: u64) -> u64 {
	previous := native_tls_file_default
	native_tls_file_default = value
	return previous
}

@(export)
native_tls_local_read_write :: proc "c" (next: u64) -> u64 {
	@(thread_local)
	local_tls: u64
	@(static)
	local_ordinary: u64

	previous := local_tls
	local_tls = next
	local_ordinary += next
	return previous + local_tls + local_ordinary
}

@(export)
native_tls_all_models :: proc "c" (value: u64) -> u64 {
	native_tls_file_default += value
	native_tls_file_globaldynamic += value + 1
	native_tls_file_localdynamic += value + 2
	native_tls_file_initialexec += value + 3
	native_tls_file_localexec += value + 4
	return native_tls_file_default +
	       native_tls_file_globaldynamic +
	       native_tls_file_localdynamic +
	       native_tls_file_initialexec +
	       native_tls_file_localexec
}

@(export)
native_tls_two_operands :: proc "c" (live: u64) -> u64 {
	lhs := native_tls_file_globaldynamic
	rhs := native_tls_file_localdynamic
	native_tls_file_globaldynamic = lhs + live
	native_tls_file_localdynamic = rhs ~ live
	return lhs + rhs + live
}

@(export)
native_tls_f64_arithmetic :: proc "c" (value, scale: f64) -> f64 {
	previous := native_tls_file_f64
	native_tls_file_f64 = previous * scale + value
	return previous + native_tls_file_f64
}

@(export)
native_tls_vector_arithmetic :: proc "c" (value: Native_TLS_Vector) -> Native_TLS_Vector {
	previous := native_tls_file_vector
	native_tls_file_vector = previous + value
	return previous - native_tls_file_vector
}

@(export)
native_tls_aggregate_offset :: proc "c" (index: uintptr, value: u64) -> u64 {
	native_tls_file_block.header += u32(index)
	native_tls_file_block.payload[index%3] = value
	offset := (^u64)(
		uintptr(&native_tls_file_block) +
		offset_of(Native_TLS_Block, payload) +
		size_of(u64),
	)
	previous := offset^
	offset^ = previous ~ value
	native_tls_file_block.value += f64(previous)
	return native_tls_file_block.payload[2] + offset^
}

@(export)
native_tls_address_escape :: proc "c" () -> uintptr {
	return native_tls_address_sink(rawptr(&native_tls_file_default))
}

@(export)
native_tls_call_survival :: proc "c" (live: u64) -> u64 {
	address := &native_tls_file_globaldynamic
	before := address^
	direct := native_tls_direct_callee(&native_tls_file_default, live)
	after_direct := address^
	call: Native_TLS_Call = native_tls_indirect_callee
	indirect := call(&native_tls_file_localexec, live + direct)
	after_indirect := address^
	return before + after_direct + after_indirect + direct + indirect + live
}

@(export)
native_tls_conditional_phi :: proc "c" (condition: bool, value: u64) -> u64 {
	address := &native_tls_file_default
	if condition {
		address = &native_tls_file_initialexec
	} else {
		address = &native_tls_file_localexec
	}
	previous := address^
	address^ = previous + value
	return address^
}

@(export)
native_tls_loop :: proc "c" (count: int, seed: u64) -> u64 {
	result := seed
	for index := 0; index < count; index += 1 {
		previous := native_tls_file_default
		native_tls_file_default = previous + result + u64(index)
		result ~= native_tls_file_default
	}
	return result
}

@(export)
native_tls_sret :: proc "c" (value: u64) -> Native_TLS_Result {
	return Native_TLS_Result {
		first  = native_tls_file_default,
		second = native_tls_file_globaldynamic + value,
		third  = native_tls_file_initialexec,
		fourth = native_tls_direct_callee(&native_tls_file_localexec, value),
	}
}

@(export)
native_tls_ordinary_controls :: proc "c" (value: u64) -> u64 {
	@(static)
	local_ordinary: u64

	previous := native_tls_ordinary_file
	native_tls_ordinary_file = value
	local_ordinary += previous + value
	return previous + native_tls_ordinary_file + local_ordinary
}

main :: proc() {}
