package main

import p "./proxy"

Params :: struct #align(16) { data: [20]f32 }
Batch :: struct { vs: Params }
Renderer :: struct { batch: Batch }

main :: proc() {
	renderer: Renderer
	p.set_expected_ptr(rawptr(&renderer.batch.vs))
	p.apply_uniforms(0, {ptr = &renderer.batch.vs, size = size_of(renderer.batch.vs)})
}
