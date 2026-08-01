package main

Result_Error :: enum u8 {None, Failed}

pair :: proc(ok: bool) -> (value: int, err: Result_Error) {
	if ok {
		return 7, .None
	}
	return 0, .Failed
}

consume :: proc(value: int, err: Result_Error, total: ^int) -> Result_Error {
	total^ += value
	return err
}

forward :: proc(ok: bool) -> (value: int, err: Result_Error) {
	total := 0
	consume(pair(ok), &total) or_return
	return total, .None
}

main :: proc() {
	forward(true)
	forward(false)
}
