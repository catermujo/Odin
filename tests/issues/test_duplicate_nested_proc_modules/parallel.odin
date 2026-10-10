package main

Try_Ctx :: struct($T, $D: typeid) {
    callback: proc(start, end: T, data: ^D) -> bool,
    data: ^D,
}
Find_Ctx :: struct($T: typeid) {
    items: []T,
    pred: proc(item: T) -> bool,
}

run_chunks :: proc(count: uint, ctx: ^$U, $run_chunk: proc(ctx: ^U, start, end: uint) -> bool) -> bool {
    return run_chunk(ctx, 0, count)
}

try_parallel_for_chunk :: proc(count: uint, $callback: proc(start, end: $T, data: ^$D) -> bool, data: ^D) -> bool {
    ctx := Try_Ctx(T, D){callback = callback, data = data}
    run_chunk :: proc(ctx: ^Try_Ctx(T, D), start, end: uint) -> bool {
        return ctx.callback(T(start), T(end), ctx.data)
    }
    return run_chunks(count, &ctx, run_chunk)
}

find_any_index :: proc(items: []$T, pred: proc(item: T) -> bool) -> (index: int, ok: bool) {
    if len(items) == 0 do return -1, false
    ctx := Find_Ctx(T){items = items, pred = pred}
    run_chunk :: proc(start, end: uint, ctx: ^Find_Ctx(T)) -> bool {
        for i := start; i < end; i += 1 {
            if ctx.pred(ctx.items[i]) do return true
        }
        return false
    }
    found := try_parallel_for_chunk(uint(len(items)), run_chunk, &ctx)
    if !found do return -1, false
    return 0, true
}

accept_i32 :: proc(item: i32) -> bool { return item > 0 }
accept_i64 :: proc(item: i64) -> bool { return item > 0 }
accept_u64 :: proc(item: u64) -> bool { return item > 0 }
accept_f64 :: proc(item: f64) -> bool { return item > 0 }

main :: proc() { assert(call_a()); assert(call_b()); assert(call_c()) }
