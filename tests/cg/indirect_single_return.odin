#+feature using-stmt

package main

Pair :: [2]i16

Range :: struct {
	start, finish: Pair,
}

Range_Key :: struct {
	epoch:      u64,
	range:      Range,
	generation: u16,
}

Prepared_Key :: struct {
	range_key:    Range_Key,
	light_serial: u64,
	expand:       bool,
}

Bounds :: struct {
	prepare_tile_min, prepare_tile_max: Pair,
}

make_range_key :: #force_inline proc(epoch: u64, start, finish: Pair, generation: u16) -> Range_Key {
	return {
		epoch = epoch,
		range = {start = start, finish = finish},
		generation = generation,
	}
}

make_key :: #force_inline proc(epoch: u64, start, finish: Pair, generation: u16, serial: u64, expand: bool) -> Prepared_Key {
	return {
		range_key = make_range_key(epoch, start, finish, generation),
		light_serial = serial,
		expand = expand,
	}
}

make_using_key :: proc(using bounds: Bounds) -> Prepared_Key {
	serial := u64(19)
	return make_key(17, prepare_tile_min, prepare_tile_max, 13, serial, true)
}

main :: proc() {
	key := make_key(17, {3, 5}, {7, 11}, 13, 19, true)
	assert(key.range_key.epoch == 17)
	assert(key.range_key.range.start == Pair{3, 5})
	assert(key.range_key.range.finish == Pair{7, 11})
	assert(key.range_key.generation == 13)
	assert(key.light_serial == 19)
	assert(key.expand)
	key = make_using_key({prepare_tile_min = {3, 5}, prepare_tile_max = {7, 11}})
	assert(key.range_key.range.start == Pair{3, 5})
	assert(key.range_key.range.finish == Pair{7, 11})
}
