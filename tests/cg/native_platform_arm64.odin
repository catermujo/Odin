package native_platform_arm64

import "base:intrinsics"

foreign {
	native_platform_foreign_sink :: proc "c" (value: uintptr) -> uintptr ---
}

when ODIN_ARCH == .arm64 {
	@(export)
	native_platform_counter_direct :: proc "c" () -> i64 {
		return intrinsics.read_cycle_counter()
	}

	@(export)
	native_platform_frequency_direct :: proc "c" () -> i64 {
		return intrinsics.read_cycle_counter_frequency()
	}

	@(export)
	native_platform_counter_duplicate_use :: proc "c" () -> i64 {
		value := intrinsics.read_cycle_counter()
		return value ~ (value << 1)
	}

	@(export)
	native_platform_counter_two_reads :: proc "c" () -> i64 {
		first := intrinsics.read_cycle_counter()
		second := intrinsics.read_cycle_counter()
		return first ~ second
	}

	@(export)
	native_platform_counter_call_survival :: proc "c" (seed: uintptr) -> i64 {
		value := intrinsics.read_cycle_counter()
		_ = native_platform_foreign_sink(seed)
		return value
	}

	@(export)
	native_platform_counter_branch :: proc "c" (condition: bool, fallback: i64) -> i64 {
		if condition {
			return intrinsics.read_cycle_counter()
		}
		return fallback
	}

	@(export)
	native_platform_counter_phi :: proc "c" (condition: bool, fallback: i64) -> i64 {
		result := fallback
		if condition {
			result = intrinsics.read_cycle_counter()
		} else {
			result ~= 1
		}
		return result
	}

	@(export)
	native_platform_counter_multiblock :: proc "c" (condition: bool, lhs, rhs: i64) -> i64 {
		value := intrinsics.read_cycle_counter()
		if condition {
			return value + lhs
		}
		return value + rhs
	}

	@(export)
	native_platform_syscall_arity_one :: proc "c" (number: uintptr) -> uintptr {
		return intrinsics.syscall(number)
	}

	@(export)
	native_platform_syscall_arity_seven :: proc "c" (number, a, b, c, d, e, f: uintptr) -> uintptr {
		return intrinsics.syscall(number, a, b, c, d, e, f)
	}

	@(export)
	native_platform_syscall_expressions :: proc "c" (number, a, b, c, d, e, f: uintptr) -> uintptr {
		return intrinsics.syscall(number ~ a, a + b, b ~ c, c + d, d ~ e, e + f, f ~ number)
	}

	@(export)
	native_platform_syscall_ordering :: proc "c" (address: ^uintptr, number, before: uintptr) -> uintptr {
		address^ = before
		result := intrinsics.syscall(number, uintptr(address))
		observed := address^
		return result ~ observed
	}

	@(export)
	native_platform_syscall_result_survival :: proc "c" (
		number, clobber: uintptr,
		condition: bool,
	) -> uintptr {
		value := intrinsics.syscall(number)
		clobbered := native_platform_foreign_sink(clobber)
		result := value
		if condition {
			result ~= clobbered
		} else {
			result += clobbered
		}
		return result
	}

	@(export)
	native_platform_inline_side_effect :: proc "c" (value: i64) -> i64 {
		return asm(i64) -> i64 #side_effects {
			"", "=r,0",
		}(value)
	}

	@(export)
	native_platform_inline_align_stack :: proc "c" (value: i64) -> i64 {
		return asm(i64) -> i64 #align_stack {
			"", "=r,0",
		}(value)
	}

	@(export)
	native_platform_inline_intel :: proc "c" (value: i64) -> i64 {
		return asm(i64) -> i64 #side_effects #intel {
			"", "=r,0",
		}(value)
	}

	@(export)
	native_platform_inline_second_parameter :: proc "c" (first, second: i64) -> i64 {
		_ = first
		return asm(i64) -> i64 #side_effects {
			"", "=r,0",
		}(second)
	}

	@(export)
	native_platform_inline_computed :: proc "c" (lhs, rhs: i64) -> i64 {
		return asm(i64) -> i64 #side_effects {
			"", "=r,0",
		}(lhs + rhs)
	}

	@(export)
	native_platform_inline_call_survival :: proc "c" (value: i64, clobber: uintptr, condition: bool) -> i64 {
		captured := asm(i64) -> i64 #side_effects {
			"", "=r,0",
		}(value)
		clobbered := i64(native_platform_foreign_sink(clobber))
		result := captured
		if condition {
			result ~= clobbered
		} else {
			result += clobbered
		}
		return result
	}

	@(export)
	native_platform_inline_load_snapshot :: proc "c" (address: ^i64, replacement: i64) -> i64 {
		captured := asm(i64) -> i64 #side_effects {
			"", "=r,0",
		}(address^)
		address^ = replacement
		return captured
	}

	@(export)
	native_platform_inline_ordering :: proc "c" (
		ordinary, watched: ^i64,
		before, after: i64,
	) -> i64 {
		ordinary^ = before
		intrinsics.volatile_store(watched, before)
		first := i64(native_platform_foreign_sink(uintptr(ordinary)))
		captured := asm(i64) -> i64 #side_effects {
			"", "=r,0",
		}(first ~ before)
		second := i64(native_platform_foreign_sink(uintptr(watched)))
		ordinary^ = after
		intrinsics.volatile_store(watched, after)
		return captured ~ second ~ (ordinary^) ~ intrinsics.volatile_load(watched)
	}
}

main :: proc() {}
