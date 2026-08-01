package main

import "core:testing"

foreign {
	test_only_symbol :: proc() ---
}

@(test)
test_only :: proc(t: ^testing.T) {
	test_only_symbol()
}

main :: proc() {
}
