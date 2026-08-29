package main
f :: proc {
    generic,
    mixed,
}
generic :: proc(a: $T/int) -> int { return 2 }
mixed :: proc(a: string) -> int { return 1 }
main :: proc() { n := 1; s := "s"; assert(f(s) == 1); assert(f(n) == 2) }
