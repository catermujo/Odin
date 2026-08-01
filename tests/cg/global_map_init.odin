#+feature dynamic-literals

package main

global_values: map[string]int = {"entry" = 7}

main :: proc() {
	if global_values["entry"] != 7 || len(global_values) != 1 {
		panic("CG global map initialization failure")
	}
}
