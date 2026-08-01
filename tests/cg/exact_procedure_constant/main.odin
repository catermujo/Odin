package main

Counter :: struct {
    calls: int,
    total: int,
}

Small_Counter :: struct {
    calls: int,
    total: u32,
}

named_int :: proc(start, end: int, data: ^Counter) {
    data.calls += 1
    data.total += start + end
}

named_u32 :: proc(start, end: u32, data: ^Small_Counter) {
    data.calls += 1
    data.total += start + end
}

named_alias_u32_a :: named_u32
named_alias_u32_b :: named_alias_u32_a

run_fixed :: proc(callback: proc(start, end: int, data: rawptr), data: rawptr) {
    callback(3, 5, data)
}

run_polymorphic :: proc($T: typeid, $D: typeid, $callback: proc(start, end: T, data: ^D), data: ^D) {
    Context :: struct {
        callback: proc(start, end: T, data: ^D),
        data:     ^D,
    }

    ctx := Context{callback, data}
    run_fixed(proc(start, end: int, ctx_raw: rawptr) {
            typed := (^Context)(ctx_raw)
            typed.callback(T(start), T(end), typed.data)
        }, rawptr(&ctx))
}

test_named :: proc() {
    data: Counter
    run_polymorphic(int, Counter, named_int, &data)
    assert(data.calls == 1)
    assert(data.total == 8)
}

test_alias :: proc() {
    data: Small_Counter
    run_polymorphic(u32, Small_Counter, named_alias_u32_b, &data)
    assert(data.calls == 1)
    assert(data.total == 8)
}

test_literal :: proc() {
    data: Counter
    run_polymorphic(int, Counter, proc(start, end: int, data: ^Counter) {
            data.calls += 1
            data.total += start * end
        }, &data)
    assert(data.calls == 1)
    assert(data.total == 15)
}

main :: proc() {
    test_named()
    test_alias()
    test_literal()
}
