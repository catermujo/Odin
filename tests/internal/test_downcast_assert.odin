package test_internal

import "core:testing"

Test_Downcast_Bit_Field :: bit_field u8 {
	a: u8   | 7,
	b: bool | 1,
}

downcast_assert_enabled_by_tag :: proc(v: u64) -> u8 #downcast_assert {
	return u8(v)
}

downcast_assert_disabled_by_tag :: proc(v: u64) -> u8 #no_downcast_assert {
	return u8(v)
}

@test
test_downcast_assert_safe_cast :: proc(t: ^testing.T) {
	x := u64(255)
	y := u64(max(i64))
	testing.expect_value(t, u8(x), u8(255))
	testing.expect_value(t, i64(y), max(i64))
}

@test
test_downcast_assert_overflow_to_smaller_int :: proc(t: ^testing.T) {
	when ODIN_EMIT_DOWNCAST_ASSERT {
		x := u64(256)
		testing.expect_assert(t, "Invalid downcast from u64 to u8 (value: 256)")
		_ = u8(x)
	} else {
		x := u64(256)
		testing.expect_value(t, u8(x), u8(0))
	}
}

@test
test_downcast_assert_overflow_same_width_signedness :: proc(t: ^testing.T) {
	when ODIN_EMIT_DOWNCAST_ASSERT {
		x := max(u64)
		testing.expect_assert(t, "Invalid downcast from u64 to i64 (value: 18446744073709551615)")
		_ = i64(x)
	} else {
		x := max(u64)
		testing.expect_value(t, i64(x), i64(-1))
	}
}

@test
test_downcast_assert_bit_field_width :: proc(t: ^testing.T) {
	ok := Test_Downcast_Bit_Field{a = u8(127), b = true}
	testing.expect_value(t, ok.a, u8(127))
	testing.expect_value(t, ok.b, true)

	when ODIN_EMIT_DOWNCAST_ASSERT {
		x := u8(128)
		testing.expect_assert(t, "Invalid downcast from u8 to u8 (value: 128)")
		_ = Test_Downcast_Bit_Field{a = x, b = true}
		testing.expect(t, false, "Expected bit_field assignment to assert for value 128")
	} else {
		x := u8(128)
		masked := Test_Downcast_Bit_Field{a = x, b = true}
		testing.expect_value(t, masked.a, u8(0))
		testing.expect_value(t, masked.b, true)
	}
}

@test
test_downcast_assert_force_enable_statement :: proc(t: ^testing.T) {
	x := u64(256)
	testing.expect_assert(t, "Invalid downcast from u64 to u8 (value: 256)")
	#downcast_assert _ = u8(x)
}

@test
test_downcast_assert_force_disable_statement :: proc(t: ^testing.T) {
	when ODIN_EMIT_DOWNCAST_ASSERT {
		x := u64(256)
		#no_downcast_assert y := u8(x)
		testing.expect_value(t, y, u8(0))
	}
}

@test
test_downcast_assert_force_enable_proc_tag :: proc(t: ^testing.T) {
	x := u64(256)
	testing.expect_assert(t, "Invalid downcast from u64 to u8 (value: 256)")
	_ = downcast_assert_enabled_by_tag(x)
}

@test
test_downcast_assert_force_disable_proc_tag :: proc(t: ^testing.T) {
	when ODIN_EMIT_DOWNCAST_ASSERT {
		x := u64(256)
		testing.expect_value(t, downcast_assert_disabled_by_tag(x), u8(0))
	}
}
