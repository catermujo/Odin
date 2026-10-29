package test_internal

import "core:testing"

@(test)
bit_field_promoted_compound_literal :: proc(t: ^testing.T) {
    Bits :: bit_field u8 {
        grab:   bool | 1,
        count:  u8   | 3,
        signed: i8   | 4,
    }
    Flags :: struct {
        using bits: Bits,
        tag:        u8,
    }
    Nested_Flags :: struct {
        using flags: Flags,
    }

    FULL :: Nested_Flags {
        grab   = true,
        count  = 5,
        signed = -3,
    }
    testing.expect_value(t, FULL.grab, true)
    testing.expect_value(t, FULL.signed, i8(-3))
    testing.expect_value(t, FULL.count, u8(5))

    PARTIAL :: Nested_Flags {
        count = 3,
        tag   = 9,
    }
    testing.expect_value(t, PARTIAL.grab, false)
    testing.expect_value(t, PARTIAL.signed, i8(0))
    testing.expect_value(t, PARTIAL.count, u8(3))
    testing.expect_value(t, PARTIAL.tag, u8(9))

    EXPLICIT :: Nested_Flags {
        flags = Flags{bits = Bits{grab = true, count = 2, signed = -1}, tag = 6},
    }
    testing.expect_value(t, EXPLICIT.grab, true)
    testing.expect_value(t, EXPLICIT.signed, i8(-1))
    testing.expect_value(t, EXPLICIT.count, u8(2))
    testing.expect_value(t, EXPLICIT.tag, u8(6))

    grab := true
    count := u8(5)
    flags := Nested_Flags {
        grab   = grab,
        count  = count,
        signed = -2,
        tag    = 4,
    }
    testing.expect_value(t, flags.grab, true)
    testing.expect_value(t, flags.signed, i8(-2))
    testing.expect_value(t, flags.count, u8(5))
    testing.expect_value(t, flags.tag, u8(4))

    Text_Fields :: struct {
        name:    string,
        score:   f64,
        pointer: ^u8,
    }
    Text_Container :: struct {
        using fields: Text_Fields,
        tag:          u8,
    }
    EMPTY_TEXT :: Text_Container {
        tag = 1,
    }
    testing.expect_value(t, EMPTY_TEXT.name, "")
    testing.expect_value(t, EMPTY_TEXT.score, 0.0)
    testing.expect_value(t, EMPTY_TEXT.pointer == nil, true)
}
