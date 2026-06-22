package test_issues

foo_int :: proc(x: int) {}
foo_bool :: proc(x: bool) {}
foo :: proc{foo_int, foo_bool}

main :: proc() {
	foo("bad")
}
