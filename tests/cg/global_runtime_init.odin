package main

seed :: proc "contextless" () -> int {
	return 40
}

seed_i32 :: proc "contextless" () -> i32 {
	return 13
}

first: int = seed() + 1
second: int = first + 1
values: [2]int = {first, second}
direct_i32: i32 = seed_i32()

@(export)
global_runtime_reader :: proc "c" () -> i32 {
	return direct_i32 + i32(first + second + values[0] + values[1])
}

main :: proc() {}
