package native_module_lifecycle

seed_i32 :: proc "contextless" () -> i32 {
	return 13
}

runtime_seed: i32 = seed_i32()
dependent_value: i32 = runtime_seed + 1
lifecycle_state: i32

@(init)
native_lifecycle_init_a :: proc "contextless" () {
	lifecycle_state = 1
}

@(init)
native_lifecycle_init_b :: proc "contextless" () {
	lifecycle_state = 2
}

@(fini)
native_lifecycle_fini_a :: proc "contextless" () {
	lifecycle_state = 3
}

@(fini)
native_lifecycle_fini_b :: proc "contextless" () {
	lifecycle_state = 4
}

native_lifecycle_dead :: proc "contextless" () {
	lifecycle_state = 99
}

@(export)
native_lifecycle_reader :: proc "c" () -> i32 {
	return dependent_value + lifecycle_state
}

main :: proc() {}
