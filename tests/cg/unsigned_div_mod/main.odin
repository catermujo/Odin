package main

unsigned_div_mod :: #force_no_inline proc(value, divisor: u64) -> (quotient, remainder, floored: u64) {
    quotient = value / divisor
    remainder = value % divisor
    floored = value %% divisor
    quotient /= 1
    remainder %= divisor
    floored %%= divisor
    return
}

signed_div_mod :: #force_no_inline proc(value, divisor: i64) -> (quotient, remainder, floored: i64) {
    quotient = value / divisor
    remainder = value % divisor
    floored = value %% divisor
    quotient /= 1
    remainder %= divisor
    floored %%= divisor
    return
}

main :: proc() {
    uquotient, uremainder, ufloored := unsigned_div_mod(0xb007885ce876993b, 0xaf7)
    assert(uquotient == 4518794730363643)
    assert(uremainder == 782)
    assert(ufloored == 782)

    squotient, s_remainder, sfloored := signed_div_mod(-7, 3)
    assert(squotient == -2)
    assert(s_remainder == -1)
    assert(sfloored == 2)
}
