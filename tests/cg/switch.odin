package main

select :: proc(value: i32) -> i32 {
	result: i32 = 0
	switch value {
	case -1:
		result = 1
	case 7:
		result = 2
	case:
		result = 3
	}
	return result
}

direct_select :: proc(value: i32) -> i32 {
	switch value {
	case -1:
		return 1
	case 7:
		return 2
	case:
		return 3
	}
}

scoped_select :: proc(value: i32) -> i32 {
	result: i32 = 4
	switch value {
	case -1:
		result: i32 = 1
		result += 1
	case:
		result = 3
	}
	return result
}

main :: proc() {
	select(-1)
	select(7)
	select(0)
	direct_select(-1)
	direct_select(7)
	direct_select(0)
	scoped_select(-1)
	scoped_select(0)
}
