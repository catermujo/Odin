package main

stop_at_three :: proc(limit: int) -> int {
	count := 0
outer: for i := 0; i < limit; i += 1 {
		switch i {
		case 3:
			break outer
		case:
			count += 1
		}
	}
	return count
}

skip_inner :: proc(limit: int) -> int {
	total := 0
outer: for i := 0; i < limit; i += 1 {
		for j := 0; j < 3; j += 1 {
			if j == 1 {
				continue outer
			}
			total += i + j
		}
	}
	return total
}

stop_range :: proc(values: []int) -> int {
	total := 0
outer: for i in values {
		if i == 3 {
			break outer
		}
		total += i
	}
	return total
}

stop_switch :: proc(value: int) -> int {
	result := 0
done: switch value {
	case 0:
		result = 1
		break done
	case:
		result = 2
	}
	return result
}

labeled_block :: proc() -> int {
	result := 0
done: {
	result = 1
	break done
}
	return result
}

labeled_if :: proc(value: int) -> int {
	result := 0
done: if value > 0 {
	result = 1
	break done
}
	return result
}

main :: proc() {
	values := []int{0, 1, 2, 3}
	_ = stop_at_three(8)
	_ = skip_inner(8)
	_ = stop_range(values)
	_ = stop_switch(0)
	_ = labeled_block()
	_ = labeled_if(1)
}
