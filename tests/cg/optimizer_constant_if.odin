package cg

@(export)
constant_if_return :: proc() -> i32 {
	condition := true
	if condition {
		return 13
	} else {
		return 29
	}
}
