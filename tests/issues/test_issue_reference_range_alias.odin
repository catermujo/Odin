package main
Item :: struct {
    value: int,
}
main :: proc() {
    items := [2]Item{{1}, {2}}
    view := items[:]
    for &item in view {
        item.value += 10
    }
    assert(items[0].value == 11)
    assert(items[1].value == 12)
}
