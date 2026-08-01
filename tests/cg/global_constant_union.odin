package main

Pair :: struct {
	left, right: i32,
}

Value :: union {i64, Pair}
Text  :: union {cstring, i64}

integer_value: Value = i64(42)
pair_value:    Value = Pair{3, 5}
text_value:    Text  = cstring("CG global")

@(export)
global_constant_union :: proc "c" () -> i64 {
	integer, integer_ok := integer_value.(i64)
	pair, pair_ok := pair_value.(Pair)
	if !integer_ok || !pair_ok {
		return -1
	}
	return integer + i64(pair.left + pair.right)
}

@(export)
global_constant_union_text :: proc "c" () -> cstring {
	text, ok := text_value.(cstring)
	if !ok {
		return nil
	}
	return text
}

main :: proc() {}
