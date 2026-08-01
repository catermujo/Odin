package main

Result_Error :: enum u8 {
	None,
	Failed,
}

fallible :: proc(ok: bool) -> (value: i32, err: Result_Error) {
	if ok {
		return 7, .None
	}
	return 0, .Failed
}

fallible_error :: proc(ok: bool) -> Result_Error {
	if ok {
		return .None
	}
	return .Failed
}

forward :: proc(ok: bool) -> (value: i32, err: Result_Error) {
	value = fallible(ok) or_return
	return value, .None
}

forward_error :: proc(ok: bool) -> (value: i32, err: Result_Error) {
	fallible_error(ok) or_return
	return 7, .None
}

main :: proc() {
	forward(true)
	forward(false)
	forward_error(true)
	forward_error(false)
}
