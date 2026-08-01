package main

float_array_calls: int
float_scalar_calls: int
event_state: int
ordered_array_calls: int
ordered_scalar_calls: int

make_float_array :: #force_no_inline proc(left, right: f32) -> [2]f32 {
    float_array_calls += 1
    return [2]f32{left, right}
}

make_float_scalar :: #force_no_inline proc(value: f32) -> f32 {
    float_scalar_calls += 1
    return value
}

make_ordered_array :: #force_no_inline proc(left, right: f32) -> [2]f32 {
    assert(event_state == 1)
    event_state = 2
    ordered_array_calls += 1
    return [2]f32{left, right}
}

make_ordered_scalar :: #force_no_inline proc(value: f32) -> f32 {
    assert(event_state == 0)
    event_state = 1
    ordered_scalar_calls += 1
    return value
}

make_signed_array :: #force_no_inline proc(left, right: i32) -> [2]i32 {
    return [2]i32{left, right}
}

make_unsigned_array :: #force_no_inline proc(left, right: u32) -> [2]u32 {
    return [2]u32{left, right}
}

exact_float_constant :: 0.31
exact_integer_constant :: 3

assert_float_array :: proc(got0, got1, want0, want1: f32) {
    assert(got0 == want0)
    assert(got1 == want1)
}

assert_signed_array :: proc(got0, got1, want0, want1: i32) {
    assert(got0 == want0)
    assert(got1 == want1)
}

assert_unsigned_array :: proc(got0, got1, want0, want1: u32) {
    assert(got0 == want0)
    assert(got1 == want1)
}

main :: proc() {
    float_array_calls = 0
    float_scalar_calls = 0
    array_array := make_float_array(2, 4) + make_float_array(1, 2)
    assert_float_array(array_array[0], array_array[1], 3, 6)
    assert(float_array_calls == 2 && float_scalar_calls == 0)

    equal_left := make_float_array(2, 4)
    equal_right := make_float_array(2, 4)
    assert(equal_left == equal_right)
    assert(!(equal_left != equal_right))
    different_first := make_float_array(1, 4)
    assert(different_first != equal_right)
    assert(!(different_first == equal_right))
    different_second := make_float_array(2, 5)
    assert(different_second != equal_right)
    assert(!(different_second == equal_right))

    float_array_calls = 0
    float_scalar_calls = 0
    array := make_float_array(2, 4)
    scalar := make_float_scalar(2)
    array_plus_scalar := array + scalar
    assert_float_array(array_plus_scalar[0], array_plus_scalar[1], 4, 6)
    array_minus_scalar := array - scalar
    assert_float_array(array_minus_scalar[0], array_minus_scalar[1], 0, 2)
    array_times_scalar := array * scalar
    assert_float_array(array_times_scalar[0], array_times_scalar[1], 4, 8)
    array_div_scalar := array / scalar
    assert_float_array(array_div_scalar[0], array_div_scalar[1], 1, 2)

    scalar_plus_array := scalar + array
    assert_float_array(scalar_plus_array[0], scalar_plus_array[1], 4, 6)
    scalar_minus_array := scalar - array
    assert_float_array(scalar_minus_array[0], scalar_minus_array[1], 0, -2)
    scalar_times_array := scalar * array
    assert_float_array(scalar_times_array[0], scalar_times_array[1], 4, 8)
    scalar_div_array := scalar / array
    assert_float_array(scalar_div_array[0], scalar_div_array[1], 1, 0.5)
    assert(float_array_calls == 1 && float_scalar_calls == 1)

    event_state = 0
    ordered_array_calls = 0
    ordered_scalar_calls = 0
    ordered := make_ordered_scalar(2) - make_ordered_array(1, 3)
    assert_float_array(ordered[0], ordered[1], 1, -1)
    assert(event_state == 2 && ordered_array_calls == 1 && ordered_scalar_calls == 1)

    float_array_calls = 0
    float_scalar_calls = 0
    array_scalar_once := make_float_array(2, 4) * make_float_scalar(3)
    assert_float_array(array_scalar_once[0], array_scalar_once[1], 6, 12)
    assert(float_array_calls == 1 && float_scalar_calls == 1)

    float_values := [2]f32{2, 4}
    float_values *= scalar
    assert_float_array(float_values[0], float_values[1], 4, 8)

    exact_float_mul := [2]f32{2, 4} * 0.31
    assert_float_array(exact_float_mul[0], exact_float_mul[1], 0.62, 1.24)
    exact_float_add := [2]f32{2, 4} + 0.5
    assert_float_array(exact_float_add[0], exact_float_add[1], 2.5, 4.5)
    exact_float_sub := [2]f32{2, 4} - 0.5
    assert_float_array(exact_float_sub[0], exact_float_sub[1], 1.5, 3.5)
    exact_float_div := [2]f32{2, 4} / 0.5
    assert_float_array(exact_float_div[0], exact_float_div[1], 4, 8)
    exact_float_scalar_add := 0.5 + [2]f32{2, 4}
    assert_float_array(exact_float_scalar_add[0], exact_float_scalar_add[1], 2.5, 4.5)
    exact_float_scalar_sub := 5.0 - [2]f32{2, 4}
    assert_float_array(exact_float_scalar_sub[0], exact_float_scalar_sub[1], 3, 1)
    exact_float_scalar_mul := 0.5 * [2]f32{2, 4}
    assert_float_array(exact_float_scalar_mul[0], exact_float_scalar_mul[1], 1, 2)
    exact_float_scalar_div := 8.0 / [2]f32{2, 4}
    assert_float_array(exact_float_scalar_div[0], exact_float_scalar_div[1], 4, 2)
    exact_named_float := [2]f32{2, 4} * exact_float_constant
    assert_float_array(exact_named_float[0], exact_named_float[1], 0.62, 1.24)
    exact_integer_to_float := [2]f32{2, 4} * 2
    assert_float_array(exact_integer_to_float[0], exact_integer_to_float[1], 4, 8)

    signed := make_signed_array(-9, 9)
    signed_scalar: i32 = 3
    signed_div := signed / signed_scalar
    assert_signed_array(signed_div[0], signed_div[1], -3, 3)
    signed_mod := signed % signed_scalar
    assert_signed_array(signed_mod[0], signed_mod[1], 0, 0)

    unsigned := make_unsigned_array(0x80000000, 9)
    unsigned_scalar: u32 = 3
    unsigned_div := unsigned / unsigned_scalar
    assert_unsigned_array(unsigned_div[0], unsigned_div[1], 0x2aaaaaaa, 3)
    unsigned_mod := unsigned % unsigned_scalar
    assert_unsigned_array(unsigned_mod[0], unsigned_mod[1], 2, 0)

    bits := make_unsigned_array(0b1010, 0b1100)
    bits_scalar: u32 = 0b0011
    bits_and := bits & bits_scalar
    assert_unsigned_array(bits_and[0], bits_and[1], 2, 0)
    bits_or := bits | bits_scalar
    assert_unsigned_array(bits_or[0], bits_or[1], 0b1011, 0b1111)
    bits_xor := bits ~ bits_scalar
    assert_unsigned_array(bits_xor[0], bits_xor[1], 0b1001, 0b1111)
    bits_and_not := bits &~ bits_scalar
    assert_unsigned_array(bits_and_not[0], bits_and_not[1], 0b1000, 0b1100)

    signed_compound := [2]i32{9, 10}
    signed_compound /= signed_scalar
    assert_signed_array(signed_compound[0], signed_compound[1], 3, 3)
    remainder_compound := [2]i32{10, 11}
    remainder_compound %= signed_scalar
    assert_signed_array(remainder_compound[0], remainder_compound[1], 1, 2)

    exact_signed_add := [2]i32{2, 4} + 3
    assert_signed_array(exact_signed_add[0], exact_signed_add[1], 5, 7)
    exact_signed_sub := 10 - [2]i32{2, 4}
    assert_signed_array(exact_signed_sub[0], exact_signed_sub[1], 8, 6)
    exact_signed_mul := [2]i32{2, 4} * 3
    assert_signed_array(exact_signed_mul[0], exact_signed_mul[1], 6, 12)
    exact_signed_div := [2]i32{6, 9} / 3
    assert_signed_array(exact_signed_div[0], exact_signed_div[1], 2, 3)
    exact_signed_scalar_div := 30 / [2]i32{6, 10}
    assert_signed_array(exact_signed_scalar_div[0], exact_signed_scalar_div[1], 5, 3)
    exact_signed_mod := [2]i32{10, 11} % 3
    assert_signed_array(exact_signed_mod[0], exact_signed_mod[1], 1, 2)
    exact_signed_scalar_mod := 10 % [2]i32{3, 4}
    assert_signed_array(exact_signed_scalar_mod[0], exact_signed_scalar_mod[1], 1, 2)

    exact_bits_and := [2]u32{0b1010, 0b1100} & 0b0011
    assert_unsigned_array(exact_bits_and[0], exact_bits_and[1], 2, 0)
    exact_bits_scalar_and := 0b0011 & [2]u32{0b1010, 0b1100}
    assert_unsigned_array(exact_bits_scalar_and[0], exact_bits_scalar_and[1], 2, 0)
    exact_bits_or := [2]u32{0b1010, 0b1100} | 0b0011
    assert_unsigned_array(exact_bits_or[0], exact_bits_or[1], 0b1011, 0b1111)
    exact_bits_scalar_or := 0b0011 | [2]u32{0b1010, 0b1100}
    assert_unsigned_array(exact_bits_scalar_or[0], exact_bits_scalar_or[1], 0b1011, 0b1111)
    exact_bits_xor := [2]u32{0b1010, 0b1100} ~ 0b0011
    assert_unsigned_array(exact_bits_xor[0], exact_bits_xor[1], 0b1001, 0b1111)
    exact_bits_scalar_xor := 0b0011 ~ [2]u32{0b1010, 0b1100}
    assert_unsigned_array(exact_bits_scalar_xor[0], exact_bits_scalar_xor[1], 0b1001, 0b1111)
    exact_bits_and_not := [2]u32{0b1010, 0b1100} &~ 0b0011
    assert_unsigned_array(exact_bits_and_not[0], exact_bits_and_not[1], 0b1000, 0b1100)
    exact_bits_scalar_and_not := 0b0011 &~ [2]u32{0b1010, 0b1100}
    assert_unsigned_array(exact_bits_scalar_and_not[0], exact_bits_scalar_and_not[1], 1, 3)
    exact_named_integer := [2]i32{6, 9} / exact_integer_constant
    assert_signed_array(exact_named_integer[0], exact_named_integer[1], 2, 3)
}
