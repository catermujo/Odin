package main

Values :: [dynamic; 4]u32

@(rodata)
values: Values = {1, 2, 3}

@(export)
global_fixed_dynamic :: proc "c" () -> u32 {
	return values[1]
}

main :: proc() {}
