package main
S :: struct {
    x: [20]int,
}
U :: union {
    S,
    bool,
}
get_value :: proc "contextless" () -> S { return S{x = 7} }
g: U = get_value()
main :: proc() { s := g.(S); for x in s.x { assert(x == 7) } }
