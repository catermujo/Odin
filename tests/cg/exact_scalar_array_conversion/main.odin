package main

Pos :: distinct [2]i16

Holder :: struct {
    plain: [2]i16,
    pos:   Pos,
}

named_zero :: 0
named_five :: 5

plain_sink_calls: int
pos_sink_calls: int

consume_plain :: #force_no_inline proc(value: [2]i16) {
    plain_sink_calls += 1
    assert(value[0] == value[1])
}

consume_pos :: #force_no_inline proc(value: Pos) {
    pos_sink_calls += 1
    assert(value[0] == value[1])
}

make_plain_zero :: proc() -> [2]i16 {
    return 0
}

make_pos_five :: proc() -> Pos {
    return 5
}

make_runtime_plain :: #force_no_inline proc() -> [2]i16 {
    return [2]i16{8, 8}
}

main :: proc() {
    plain_sink_calls = 0
    pos_sink_calls = 0

    consume_plain(0)
    consume_plain(named_zero)
    consume_plain([2]i16{})
    consume_plain([2]i16{1, 1})
    consume_plain(make_runtime_plain())

    consume_pos(0)
    consume_pos(5)
    consume_pos(named_five)

    plain: [2]i16 = 0
    plain = named_five
    consume_plain(plain)

    pos: Pos = 0
    pos = 4
    consume_pos(pos)

    returned_plain := make_plain_zero()
    assert(returned_plain[0] == 0 && returned_plain[1] == 0)
    consume_plain(returned_plain)

    returned_pos := make_pos_five()
    assert(returned_pos[0] == 5 && returned_pos[1] == 5)
    consume_pos(returned_pos)

    typed_scalar: i16 = 6
    consume_plain(typed_scalar)

    explicit := [2]i16{9, 9}
    consume_pos(Pos(explicit))

    holder := Holder {
        plain = 0,
        pos   = named_five,
    }
    assert(holder.plain[0] == 0 && holder.plain[1] == 0)
    assert(holder.pos[0] == 5 && holder.pos[1] == 5)
    consume_plain(holder.plain)
    consume_pos(holder.pos)

    assert(plain_sink_calls == 9)
    assert(pos_sink_calls == 7)
}
