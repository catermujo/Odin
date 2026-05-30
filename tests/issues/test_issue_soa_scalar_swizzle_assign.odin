package main
main :: proc() {
    s: #soa[1][3]f32
    s[0] = [3]f32{1, 2, 3}
    s[0].xy += 2
    assert(s[0] == [3]f32{3, 4, 3})
}
