package main

when_local :: proc(value: i32) -> i32 {
	when true {
		selected := value + 1
	} else {
		selected := value - 1
	}
	return selected
}

main :: proc() {
	_ = when_local(41)
}
