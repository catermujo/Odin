package main

Color :: [4]f32
WHITE: Color : 1.
Inner :: struct {alpha, invalid: f32}
Outer :: struct {inner: Inner}
g: ^Outer

main :: proc() {
	alpha := f32(0.25)
	state := Outer{inner = {alpha = alpha, invalid = .9}}
	g = &state
	valid := true
	tint := WHITE * g.inner.alpha if valid else Color{1, .45, .45, g.inner.invalid}
	assert(tint == {alpha, alpha, alpha, alpha})
}
