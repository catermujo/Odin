package main

init_state: int

dead_00 :: proc() -> int { return 0 }
dead_01 :: proc() -> int { return 1 }
dead_02 :: proc() -> int { return 2 }
dead_03 :: proc() -> int { return 3 }
dead_04 :: proc() -> int { return 4 }
dead_05 :: proc() -> int { return 5 }
dead_06 :: proc() -> int { return 6 }
dead_07 :: proc() -> int { return 7 }
dead_08 :: proc() -> int { return 8 }
dead_09 :: proc() -> int { return 9 }
dead_10 :: proc() -> int { return 10 }
dead_11 :: proc() -> int { return 11 }
dead_12 :: proc() -> int { return 12 }
dead_13 :: proc() -> int { return 13 }
dead_14 :: proc() -> int { return 14 }
dead_15 :: proc() -> int { return 15 }

@(init)
initialize :: proc "contextless" () {
	init_state = 1
	global_callback = callback
}

@(fini)
finalize :: proc "contextless" () {
	init_state = 0
}

callback :: proc(value: int) -> int {
	return value + 1
}

global_callback: proc(int) -> int

make_closure :: proc(delta: int) {
	closure := lambda [delta](value: int) -> int {
		return value + delta
	}
	_ = closure
}

@(export)
lazy_procedure_materialization_witness :: proc() -> int {
	make_closure(1)
	return callback(41)
}

main :: proc() {
	assert(lazy_procedure_materialization_witness() == 42)
}
