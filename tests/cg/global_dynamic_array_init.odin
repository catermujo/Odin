#+feature dynamic-literals

package main

global_values: [dynamic]int = {1, 2, 3}

main :: proc() {
	if len(global_values) != 3 || global_values[0] != 1 || global_values[2] != 3 {
		panic("CG global dynamic array initialization failure")
	}
}
