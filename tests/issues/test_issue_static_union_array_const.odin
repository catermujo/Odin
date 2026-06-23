package test_issues

Inner_Small :: struct {
	x: u8,
}

Inner_Big :: struct {
	x: [13]u64,
}

Token :: union {
	Inner_Small,
	Inner_Big,
}

Atom :: struct {
	token: Token,
}

main :: proc() {
	@(static) atoms := [?]Atom{
		{token = Inner_Small{1}},
	}
	_ = atoms
}
