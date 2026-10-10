#+test
package test_issues

import "core:testing"

@(test)
map_dynamic_array_range :: proc(t: ^testing.T) {
	values := make(map[[2]i64][dynamic]int)
	defer delete(values)
	count := 0
	for value in values[[2]i64{0, 0}] {
		count += value + 1
	}
	#reverse for value in values[[2]i64{0, 0}] {
		count += value + 1
	}
	testing.expect_value(t, count, 0)
	testing.expect_value(t, len(values), 0)
	items := make([dynamic]int, 0, 3)
	defer delete(items)
	append(&items, 2, 3, 5)
	values[[2]i64{1, 2}] = items
	key_calls := 0
	key := proc(calls: ^int) -> [2]i64 {
		calls^ += 1
		return {1, 2}
	}
	total := 0
	for value, index in values[key(&key_calls)] {
		total += value + index
	}
	testing.expect_value(t, total, 13)
	testing.expect_value(t, key_calls, 1)
	total = 0
	#reverse for value, index in values[[2]i64{1, 2}] {
		total = total * 10 + value
		testing.expect_value(t, value, items[index])
	}
	testing.expect_value(t, total, 532)
	for &value in values[[2]i64{1, 2}] {
		value += 1
	}
	testing.expect_value(t, items[0], 3)
	testing.expect_value(t, items[2], 6)
}

@(test)
map_inline_array_range :: proc(t: ^testing.T) {
	arrays := make(map[int][3]int)
	defer delete(arrays)
	count := 0
	for value, index in arrays[7] {
		testing.expect_value(t, value, 0)
		testing.expect_value(t, index, count)
		count += 1
	}
	testing.expect_value(t, count, 3)
	arrays[7] = {2, 3, 5}
	total := 0
	#reverse for value in arrays[7] {
		total = total * 10 + value
	}
	testing.expect_value(t, total, 532)
	Axis :: enum { x, y }
	enumerated := make(map[int][Axis]int)
	defer delete(enumerated)
	count = 0
	for value in enumerated[7] {
		testing.expect_value(t, value, 0)
		count += 1
	}
	testing.expect_value(t, count, 2)
	enumerated[7] = {2, 3}
	total = 0
	for value, axis in enumerated[7] {
		total += value + int(axis)
	}
	testing.expect_value(t, total, 6)
	fixed := make(map[int][dynamic; 3]int)
	defer delete(fixed)
	for value in fixed[7] {
		testing.expect(t, false)
		_ = value
	}
	#reverse for value in fixed[7] {
		testing.expect(t, false)
		_ = value
	}
	local: [dynamic; 3]int
	append(&local, 2, 3, 5)
	fixed[7] = local
	total = 0
	for value in fixed[7] {
		total += value
	}
	testing.expect_value(t, total, 10)
}

@(test)
map_nested_container_range :: proc(t: ^testing.T) {
	maps := make(map[int]map[int]int)
	defer delete(maps)
	count := 0
	for _, value in maps[7] {
		count += value + 1
	}
	testing.expect_value(t, count, 0)
	inner := make(map[int]int)
	defer delete(inner)
	inner[2] = 3
	maps[7] = inner
	for _, &value in maps[7] {
		value += 1
	}
	testing.expect_value(t, inner[2], 4)
	Pair :: struct { x, y: int }
	soas := make(map[int]#soa[dynamic]Pair)
	defer delete(soas)
	for pair in soas[7] {
		count += pair.x + pair.y + 1
	}
	testing.expect_value(t, count, 0)
	soa := make(#soa[dynamic]Pair, 2)
	defer delete(soa)
	soa[0] = {2, 3}
	soa[1] = {5, 7}
	soas[7] = soa
	total := 0
	for pair in soas[7] {
		total += pair.x + pair.y
	}
	testing.expect_value(t, total, 17)
}
