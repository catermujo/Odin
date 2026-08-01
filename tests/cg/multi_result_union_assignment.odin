package main

Source_Error :: enum i32 {
	None,
	Failed,
}

Destination_Error :: union #shared_nil {
	Source_Error,
}

pair :: proc() -> (i32, Source_Error) {
	return 42, .Failed
}

assign_pair :: proc() -> Destination_Error {
	_, err := pair()
	return err
}

main :: proc() {
	_ = assign_pair()
}
