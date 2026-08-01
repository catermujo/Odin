package main

import "base:intrinsics"

square_root :: proc "contextless" (value: f32) -> f32 {
	return intrinsics.sqrt(value)
}

main :: proc() {
	square_root(4)
}
