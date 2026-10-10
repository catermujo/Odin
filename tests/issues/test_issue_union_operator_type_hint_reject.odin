package test_issues

main :: proc() {
	value: Maybe(int) = "not a number" * 2
	_ = value
}
