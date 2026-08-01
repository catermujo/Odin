#+feature dynamic-literals

package main

first:  [dynamic]int = {1, 2}
second: [dynamic]int = {3, 4, 5}

main :: proc() {
	first, second = second, first
	if len(first) != 3 || first[0] != 3 || len(second) != 2 || second[1] != 2 {
		panic("CG dynamic array multi-assignment failure")
	}
}
