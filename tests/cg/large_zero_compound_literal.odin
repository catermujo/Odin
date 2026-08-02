package main

Keyed_Large_Zero :: struct {
	first:   u32,
	omitted: [16*1024]u32,
	last:    u32,
}

Positional_Large_Zero :: struct {
	first:   u32,
	middle:  u32,
	last:    u32,
	omitted: [16*1024]u32,
}

identity :: proc(value: u32) -> u32 {
	return value
}

main :: proc() {
	keyed_first: u32 = 17
	keyed := Keyed_Large_Zero{
		first = identity(keyed_first),
		last  = 29,
	}
	assert(keyed.first == 17)
	assert(keyed.last == 29)
	assert(keyed.omitted[0] == 0)
	assert(keyed.omitted[16383] == 0)

	positional_first: u32 = 31
	positional := Positional_Large_Zero{identity(positional_first), 37, 41, {}}
	assert(positional.first == 31)
	assert(positional.middle == 37)
	assert(positional.last == 41)
	assert(positional.omitted[0] == 0)
	assert(positional.omitted[16383] == 0)
}
