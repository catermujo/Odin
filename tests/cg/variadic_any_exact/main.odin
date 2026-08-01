package main

import "core:fmt"

any_string :: proc(value: any, expected: string) -> bool {
    actual, ok := value.(string)
    return ok && actual == expected
}

any_int :: proc(value: any, expected: int) -> bool {
    actual, ok := value.(int)
    return ok && actual == expected
}

any_float :: proc(value: any, expected: f64) -> bool {
    actual, ok := value.(f64)
    return ok && actual == expected
}

any_bool :: proc(value: any, expected: bool) -> bool {
    actual, ok := value.(bool)
    return ok && actual == expected
}

format_runtime_string :: proc(value: string) -> string {
    return fmt.aprint(value, ".", "tail", sep = "")
}

inspect_any :: proc(args: ..any, label := "") -> bool {
    if len(args) != 5 {
        return false
    }
    switch label {
    case "first":
        return(
            any_string(args[0], "first") &&
            any_string(args[1], "runtime") &&
            any_int(args[2], 7) &&
            any_float(args[3], 2.5) &&
            any_bool(args[4], false) \
        )
    case "middle":
        return(
            any_string(args[0], "runtime") &&
            any_string(args[1], "middle") &&
            any_int(args[2], 7) &&
            any_float(args[3], 2.5) &&
            any_bool(args[4], false) \
        )
    case "last":
        return(
            any_string(args[0], "runtime") &&
            any_int(args[1], 7) &&
            any_float(args[2], 2.5) &&
            any_bool(args[3], false) &&
            any_string(args[4], "last") \
        )
    }
    return false
}

main :: proc() {
    runtime_value := "runtime"
    formatted := format_runtime_string(runtime_value)
    assert(formatted == "runtime.tail")
    delete(formatted)

    assert(inspect_any("first", runtime_value, 7, 2.5, false, label = "first"))
    assert(inspect_any(runtime_value, "middle", 7, 2.5, false, label = "middle"))
    assert(inspect_any(runtime_value, 7, 2.5, false, "last", label = "last"))
}
