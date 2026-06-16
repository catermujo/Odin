package main
import "base:runtime"
import "core:fmt"
count: int
@(instrumentation_enter)
enter :: proc "contextless" (p, parent: rawptr, loc: runtime.Source_Code_Location) {
    if loc.procedure == "default_hasher_string" { count += 1 }
}
@(instrumentation_exit)
exit :: proc "contextless" (p, parent: rawptr, loc: runtime.Source_Code_Location) {  }
main :: proc() {
    m := make(map[string]int)
    m["x"] = 1
    _ = m["x"]
    delete(m)
    fmt.println(count)
    assert(count == 0)
}
