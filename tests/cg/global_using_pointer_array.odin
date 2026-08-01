#+feature using-stmt
package main

State :: struct {
	sheet: struct {
		terra: struct {
			sprites: [4]u64,
		},
	},
}

g: ^State

draw :: proc(index: u8) -> u64 {
	using terra := &g.sheet.terra
	assert(index < len(sprites))
	return sprites[index]
}

draw_local :: proc(index: u8) -> u64 {
	terra := &g.sheet.terra
	assert(index < len(terra.sprites))
	return terra.sprites[index]
}

draw_direct :: proc(index: u8) -> u64 {
	assert(index < len(g.sheet.terra.sprites))
	return g.sheet.terra.sprites[index]
}

main :: proc() {
	state: State
	g = &state
	g.sheet.terra.sprites[2] = 0x1234
	assert(draw_direct(2) == 0x1234)
	assert(draw_local(2) == 0x1234)
	assert(draw(2) == 0x1234)
}
