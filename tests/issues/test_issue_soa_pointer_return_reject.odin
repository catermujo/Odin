// A local #soa value must not escape its procedure frame.
package test_issues

Tile :: struct {
	value: int,
}

Chunk :: struct {
	using tiles: #soa[4]struct {
		city: Tile,
	},
}

tile_ref :: proc() -> ^Tile {
	chunk: Chunk
	return &chunk.city[0]
}

main :: proc() {
	_ = tile_ref()
}
