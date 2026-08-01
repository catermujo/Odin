package main

foreign {
	@(link_name="abs") c_abs :: proc(value: int) -> int ---
}

foreign_block :: proc(value: int) -> int {
	return c_abs(value)
}

main :: proc() {
	_ = foreign_block(-1)
}
