// Pointer-backed #soa fields are safe to return by address.
package test_issues

Tile :: struct {
	value: int,
}

Chunk :: struct {
	using tiles: #soa[4]struct {
		city: Tile,
	},
}

tile_ref :: proc(chunk: ^Chunk, index: int) -> ^Tile {
	return &chunk.city[index]
}

tile_ref_through_deref :: proc(chunk: ^Chunk, index: int) -> ^Tile {
	return &chunk^.city[index]
}

main :: proc() {
	chunk: Chunk
	tile_ref(&chunk, 0).value = 42
	tile_ref_through_deref(&chunk, 1).value = 24
}
