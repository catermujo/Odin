package main

Byte_Pair :: [2]u8
Word_Pair :: [2]u32
Signed_Byte_Pair :: [2]i8
Signed_Word_Pair :: [2]i32

to_words :: #force_no_inline proc(bytes: Byte_Pair) -> Word_Pair {
	return Word_Pair(bytes)
}

to_signed_words :: #force_no_inline proc(bytes: Signed_Byte_Pair) -> Signed_Word_Pair {
	return Signed_Word_Pair(bytes)
}

main :: proc() {
	words := to_words({1, 1})
	assert(words == Word_Pair{1, 1})

	signed_words := to_signed_words({-1, 2})
	assert(signed_words == Signed_Word_Pair{-1, 2})
}
