package main

@(export)
inline_asm_identity :: proc "contextless" (value: i64) -> i64 {
	return asm(i64) -> i64 #side_effects {
		"", "=r,0",
	}(value)
}

@(export)
inline_asm_align_stack :: proc "contextless" (value: i64) -> i64 {
	return asm(i64) -> i64 #align_stack {
		"", "=r,0",
	}(value)
}

@(export)
inline_asm_intel :: proc "contextless" (value: i64) -> i64 {
	return asm(i64) -> i64 #side_effects #intel {
		"", "=r,0",
	}(value)
}

main :: proc() {}
