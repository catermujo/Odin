package main

Kind :: enum {a, b, c}
Offset_Kind :: enum {a = 10, b, c}

main :: proc() {
	values: [Kind]i64 = {1, 2, 3}
	negative := -values
	assert(negative == {-1, -2, -3})

	offset_values: [Offset_Kind]i64 = {1, 2, 3}
	assert(offset_values[Offset_Kind.a] == 1)
	assert(offset_values[Offset_Kind.b] == 2)
	assert(offset_values[Offset_Kind.c] == 3)

	partial: [Kind]i64 = #partial {Kind.a = 1}
	assert(partial[Kind.a] == 1)
	assert(partial[Kind.b] == 0)
}
