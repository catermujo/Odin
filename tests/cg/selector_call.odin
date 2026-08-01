package main

Event :: struct {
	first: int,
	second: int,
	tick: int,
}

make_event :: proc(tick: int) -> Event {
	return Event {
		first = 1,
		second = 2,
		tick = tick,
	}
}

event_tick :: proc(tick: int) -> int {
	return make_event(tick).tick
}

main :: proc() {
	_ = event_tick(1)
}
