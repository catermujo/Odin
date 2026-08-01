package main

Uniforms :: struct #align(16) {
	using _: struct #packed {
		direction: [2]f32,
		radius:    f32,
		_:         [4]u8,
	},
}

compound_literal_using :: proc(x, y, radius: f32) -> f32 {
	uniforms := Uniforms{
		direction = {x, y},
		radius    = radius,
	}
	return uniforms.direction[0] + uniforms.direction[1] + uniforms.radius
}

main :: proc() {
	_ = compound_literal_using(1, 2, 3)
}
