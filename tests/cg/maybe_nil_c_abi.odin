package main

maybe_nil_c_abi :: proc "c" (value: Maybe(^int)) -> bool {
	return value == nil
}

main :: proc() {
	assert(maybe_nil_c_abi(nil))
}
