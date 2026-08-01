package main

import "core:strings"

make_csv :: #force_no_inline proc() -> string {
    bytes := make([]u8, 16)
    text := "alpha,beta,gamma"
    for i in 0 ..< len(text) {
        bytes[i] = text[i]
    }
    return string(bytes)
}

make_numbers :: #force_no_inline proc() -> []int {
    numbers := make([]int, 5)
    for i in 0 ..< len(numbers) {
        numbers[i] = 10 + i
    }
    return numbers
}

make_dynamic :: #force_no_inline proc() -> [dynamic]int {
    numbers: [dynamic]int
    for i in 0 ..< 5 {
        append(&numbers, 20 + i)
    }
    return numbers
}

main :: proc() {
    source := make_csv()
    all := (&source)[:]
    from_low := (&source)[1:]
    to_high := (&source)[:5]
    middle := (&source)[1:5]
    assert(all == "alpha,beta,gamma")
    assert(from_low == "lpha,beta,gamma")
    assert(to_high == "alpha")
    assert(middle == "lpha")

    source = (&source)[6:]
    assert(source == "beta,gamma")

    fixed := [4]int{1, 2, 3, 4}
    fixed_slice := (&fixed)[1:3]
    assert(len(fixed_slice) == 2)
    assert(fixed_slice[0] == 2)
    fixed[1] = 99
    assert(fixed_slice[0] == 99)

    numbers := make_numbers()
    number_slice := (&numbers)[1:4]
    assert(uintptr(raw_data(number_slice)) == uintptr(raw_data(numbers)) + size_of(int))
    assert(len(number_slice) == 3)
    assert(number_slice[0] == 11)
    numbers[1] = 99
    assert(number_slice[0] == 99)

    owner := make_dynamic()
    owner_cap := cap(owner)
    owner_slice := (&owner)[1:4]
    assert(uintptr(raw_data(owner_slice)) == uintptr(raw_data(owner)) + size_of(int))
    assert(len(owner_slice) == 3)
    assert(owner_slice[0] == 21)
    owner[1] = 199
    assert(owner_slice[0] == 199)
    assert(cap(owner) == owner_cap)

    iterator_source := make_csv()
    expected := [3]string{"alpha", "beta", "gamma"}
    count := 0
    for token in strings.split_iterator(&iterator_source, ",") {
        assert(count < len(expected))
        assert(token == expected[count])
        count += 1
    }
    assert(count == 3)
    assert(iterator_source == "")

    no_separator := (make_csv())[:5]
    token, ok := strings.split_iterator(&no_separator, ",")
    assert(ok)
    assert(token == "alpha")
    assert(no_separator == "")
    _, ok = strings.split_iterator(&no_separator, ",")
    assert(!ok)
}
