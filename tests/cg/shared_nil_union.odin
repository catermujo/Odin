package main

Local_Error :: enum u8 {
	None,
	Failed,
}

Other_Error :: enum u8 {
	None,
	Unknown,
}

Outer_Error :: union #shared_nil {
	Local_Error,
	Other_Error,
}

source :: proc(fail: bool) -> Local_Error {
	if fail {
		return .Failed
	}
	return .None
}

forward :: proc(fail: bool) -> (value: int, err: Outer_Error) {
	source(fail) or_return
	return 7, nil
}

assign :: proc(fail: bool) -> Outer_Error {
	err: Outer_Error
	err = source(fail)
	return err
}

main :: proc() {
	_, _ = forward(false)
	_, _ = forward(true)
	_ = assign(false)
	_ = assign(true)
}
