package native_linux_arm64

import "base:intrinsics"

Linux_Vector :: #simd[4]f32

Linux_Block :: struct {
	word:  u64,
	real:  f64,
	bytes: [3]u32,
}

Linux_Sret :: struct {
	values: [4]u64,
}

Linux_Indirect_Call :: proc "c" (value: u64) -> u64
Linux_TLS_Call :: proc "c" (address: ^u64, value: u64) -> u64

foreign {
	native_linux_foreign_fixed :: proc "c" (value: u64) -> u64 ---
	native_linux_foreign_vararg :: proc "c" (fixed: u64, #c_vararg args: ..any) -> u64 ---
	native_linux_foreign_address :: proc "c" (address: rawptr) -> uintptr ---
}

when ODIN_OS == .Linux && ODIN_ARCH == .arm64 {
	@(rodata)
	native_linux_readonly_data: u64 = 0x1020304050607080

	@(export)
	native_linux_exported_data: u64 = 0x8877665544332211

	@(linkage="weak")
	native_linux_hidden_weak_data: u64

	native_linux_bss_data: u64

	@(thread_local)
	native_linux_tls_default: u64
	@(thread_local="globaldynamic")
	native_linux_tls_globaldynamic: u64
	@(thread_local="localdynamic")
	native_linux_tls_localdynamic: u64
	@(thread_local="initialexec")
	native_linux_tls_initialexec: u64
	@(thread_local="localexec")
	native_linux_tls_localexec: u64
	@(thread_local)
	native_linux_tls_real: f64
	@(thread_local)
	native_linux_tls_vector: Linux_Vector
	@(thread_local)
	native_linux_tls_block: Linux_Block

	native_linux_direct_callee :: proc "c" (value: u64) -> u64 {
		return value ~ 0x5a5aa5a5f0f00f0f
	}

	native_linux_tls_direct_callee :: proc "c" (address: ^u64, value: u64) -> u64 {
		return address^ + value
	}

	native_linux_tls_indirect_callee :: proc "c" (address: ^u64, value: u64) -> u64 {
		return address^ ~ value
	}

	native_linux_vararg_register_body :: #force_no_inline proc "c" (
		fixed: u64,
		#c_vararg args: ..any,
	) -> u64 {
		list: intrinsics.c_va_list
		copy: intrinsics.c_va_list
		intrinsics.c_va_start(&list, args)
		intrinsics.c_va_copy(&copy, &list)
		integer := intrinsics.c_va_arg(&list, u64)
		real := intrinsics.c_va_arg(&list, f64)
		copied_integer := intrinsics.c_va_arg(&copy, u64)
		intrinsics.c_va_end(&copy)
		intrinsics.c_va_end(&list)
		return fixed + integer + copied_integer + u64(real)
	}

	native_linux_vararg_exhausted_body :: #force_no_inline proc "c" (
		a0, a1, a2, a3, a4, a5, a6, a7: u64,
		f0, f1, f2, f3, f4, f5, f6, f7: f64,
		#c_vararg args: ..any,
	) -> u64 {
		list: intrinsics.c_va_list
		intrinsics.c_va_start(&list, args)
		integer := intrinsics.c_va_arg(&list, u64)
		real := intrinsics.c_va_arg(&list, f64)
		trailing_integer := intrinsics.c_va_arg(&list, u64)
		intrinsics.c_va_end(&list)
		return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + u64(f0+f1+f2+f3+f4+f5+f6+f7) + integer + u64(real) + trailing_integer
	}

	@(export)
	native_linux_symbols_and_calls :: proc "c" (value: u64) -> u64 {
		native_linux_bss_data = value
		native_linux_hidden_weak_data += value
		procedure_address := native_linux_direct_callee
		direct := native_linux_direct_callee(value)
		indirect := procedure_address(value + direct)
		foreign_value := native_linux_foreign_fixed(indirect)
		address := native_linux_foreign_address(rawptr(&native_linux_exported_data))
		return native_linux_readonly_data + native_linux_exported_data + native_linux_bss_data +
		       native_linux_hidden_weak_data + direct + indirect + foreign_value + u64(address)
	}

	@(export)
	native_linux_tls_all_models :: proc "c" (value: u64) -> u64 {
		native_linux_tls_default += value
		native_linux_tls_globaldynamic += value + 1
		native_linux_tls_localdynamic += value + 2
		native_linux_tls_initialexec += value + 3
		native_linux_tls_localexec += value + 4
		return native_linux_tls_default +
		       native_linux_tls_globaldynamic +
		       native_linux_tls_localdynamic +
		       native_linux_tls_initialexec +
		       native_linux_tls_localexec
	}

	@(export)
	native_linux_tls_f64_arithmetic :: proc "c" (value, scale: f64) -> f64 {
		previous := native_linux_tls_real
		native_linux_tls_real = previous * scale + value
		return previous + native_linux_tls_real
	}

	@(export)
	native_linux_tls_vector_arithmetic :: proc "c" (value: Linux_Vector) -> Linux_Vector {
		previous := native_linux_tls_vector
		native_linux_tls_vector = previous + value
		return previous - native_linux_tls_vector
	}

	@(export)
	native_linux_tls_aggregate_offset :: proc "c" (index: uintptr, value: u64) -> u64 {
		native_linux_tls_block.bytes[index%3] = u32(value)
		offset := (^u64)(
			uintptr(&native_linux_tls_block) +
			offset_of(Linux_Block, word),
		)
		previous := offset^
		offset^ = previous ~ value
		native_linux_tls_block.real += f64(previous)
		return native_linux_tls_block.word + offset^
	}

	@(export)
	native_linux_tls_address_escape :: proc "c" () -> uintptr {
		return native_linux_foreign_address(rawptr(&native_linux_tls_default))
	}

	@(export)
	native_linux_tls_conditional_phi :: proc "c" (condition: bool, value: u64) -> u64 {

		address := &native_linux_tls_default
		if condition {
			address = &native_linux_tls_initialexec
		} else {
			address = &native_linux_tls_localexec
		}
		previous := address^
		address^ = previous + value
		return address^
	}

	@(export)
	native_linux_tls_loop :: proc "c" (count: int, seed: u64) -> u64 {
		result := seed
		for index := 0; index < count; index += 1 {
			previous := native_linux_tls_default
			native_linux_tls_default = previous + result + u64(index)
			result ~= native_linux_tls_default
		}
		return result
	}

	@(export)
	native_linux_tls_call_survival :: proc "c" (live: u64) -> u64 {
		address := &native_linux_tls_globaldynamic
		before := address^
		direct := native_linux_tls_direct_callee(&native_linux_tls_default, live)
		after_direct := address^
		call: Linux_TLS_Call = native_linux_tls_indirect_callee
		indirect := call(&native_linux_tls_localexec, live + direct)
		after_indirect := address^
		return before + after_direct + after_indirect + direct + indirect + live
	}

	@(export)
	native_linux_tls_sret :: proc "c" (value: u64) -> Linux_Sret {
		return Linux_Sret {
			values = {
				native_linux_tls_default,
				native_linux_tls_globaldynamic + native_linux_tls_localdynamic,
				native_linux_tls_initialexec + native_linux_tls_localexec,
				native_linux_tls_direct_callee(&native_linux_tls_localexec, value),
			},
		}
	}

	@(export)
	native_linux_fixed_stack :: proc "c" (
		a0, a1, a2, a3, a4, a5, a6, a7: u64,
		narrow: i8,
		half: u16,
		word: u32,
		f0, f1, f2, f3, f4, f5, f6, f7: f64,
		real16: f16,
		real32: f32,
		vector: Linux_Vector,
		block: Linux_Block,
	) -> u64 {
		_ = vector + vector
		return a0+a1+a2+a3+a4+a5+a6+a7 + u64(narrow) + u64(half) + u64(word) +
		       u64(f0+f1+f2+f3+f4+f5+f6+f7) + u64(real16) + u64(real32) + block.word + u64(block.real)
	}

	@(export)
	native_linux_fixed_stack_call :: proc "c" (seed: u64, vector: Linux_Vector) -> u64 {
		block := Linux_Block {word = seed, real = f64(seed), bytes = {1, 2, 3}}
		return native_linux_fixed_stack(
			seed, 1, 2, 3, 4, 5, 6, 7,
			-1, 0xcdef, 0x89abcdef,
			0, 1, 2, 3, 4, 5, 6, 7,
			f16(1.5), f32(2.5), vector, block,
		)
	}

	@(export)
	native_linux_varargs :: proc "c" (seed: u64) -> u64 {
		registers := native_linux_vararg_register_body(seed, i8(-4), f32(3.5))
		exhausted := native_linux_vararg_exhausted_body(
			0, 1, 2, 3, 4, 5, 6, 7,
			0, 1, 2, 3, 4, 5, 6, 7,
			u64(0x1122334455667788), f64(9.5), u64(0x8877665544332211),
		)
		foreign_direct := native_linux_foreign_vararg(seed, u8(3), f32(2.5), u64(9))
		foreign_indirect := native_linux_foreign_vararg
		return registers + exhausted + foreign_direct + foreign_indirect(seed, bool(true), f32(1.25), u64(7))
	}

	@(export)
	native_linux_syscalls :: proc "c" (number, a, b, c, d, e, f: uintptr) -> uintptr {
		first := intrinsics.syscall(number)
		second := intrinsics.syscall(number+a, a, b, c, d, e, f)
		return first ~ second
	}
} else {
	@(export)
	native_linux_darwin_control :: proc "c" (value: u64) -> u64 {
		return value + 1
	}
}

main :: proc() {}
