package test_issues

Inner_Left :: enum {
	a,
	b,
}

Inner_Right :: enum {
	c,
	d,
}

Inner :: union {
	Inner_Left,
	Inner_Right,
}

Outer :: union {
	Inner,
	int,
}

Atom :: struct {
	token: Outer,
}

NAMED_INNER :: Inner(Inner_Left.a)

DIRECT_ATOMS :: [?]Atom{
	{token = Inner(Inner_Left.a)},
}

NAMED_ATOMS :: [?]Atom{
	{token = NAMED_INNER},
}

main :: proc() {
	_ = DIRECT_ATOMS
	_ = NAMED_ATOMS
}
