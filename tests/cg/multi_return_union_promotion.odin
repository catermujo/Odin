package main

Source_Error :: enum u8 {
	None,
	Failed,
}

Return_Error :: union #shared_nil {
	Source_Error,
}

source :: proc() -> (int, Source_Error) {
	return 7, .Failed
}

forward :: proc() -> (int, Return_Error) {
	return source()
}

main :: proc() {
	value, err := forward()
	_ = value
	_ = err
}
