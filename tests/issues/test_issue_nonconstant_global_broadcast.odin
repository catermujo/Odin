package main
import "core:fmt"
get_value :: proc "contextless" () -> int { return 7 }
g: [20]int = get_value()
main :: proc() { fmt.println(g); for x in g { assert(x == 7) } }
