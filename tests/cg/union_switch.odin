package cg

Error_Kind :: enum {
	None,
	eof,
}

Other_Error :: enum {
	None,
	bad,
}

Error :: union #shared_nil {
	Error_Kind,
	Other_Error,
}

classify_error :: proc(err: Error) -> int {
	switch err {
	case nil, .eof:
		return 0
	case:
		return 1
	}
}

main :: proc() {
	assert(classify_error(nil) == 0)
	assert(classify_error(.eof) == 0)
	assert(classify_error(.bad) == 1)
}
