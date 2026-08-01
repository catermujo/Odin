#+feature dynamic-literals

package main

import "core:testing"

@(test)
map_index_len_expect :: proc(t: ^testing.T) {
	values := make(map[int][dynamic]int)
	defer delete(values)
	value := make([dynamic]int)
	defer delete(value)
	append(&value, 1)
	values[1] = value
	testing.expect_value(t, len(values[1]), 1)
	testing.expect(t, cap(values[1]) >= 1)
	testing.expect_value(t, len(values[2]), 0)

	fixed: [dynamic; 4]int
	append(&fixed, 1)
	fixed_values := make(map[int][dynamic; 4]int)
	defer delete(fixed_values)
	fixed_values[1] = fixed
	testing.expect_value(t, len(fixed_values[1]), 1)
	testing.expect_value(t, cap(fixed_values[1]), 4)

	nested_values := make(map[int]map[int]int)
	defer delete(nested_values)
	nested := make(map[int]int)
	defer delete(nested)
	nested[2] = 3
	nested_values[1] = nested
	testing.expect_value(t, len(nested_values[1]), 1)
}
