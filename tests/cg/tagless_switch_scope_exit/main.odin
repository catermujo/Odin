package main

finish :: proc(result: ^int, token: int) {
	result^ = result^*10 + token
}

begin :: proc(result: ^int, token: int) -> (handle: int) \
#scope_exit(.implicit, finish(result, handle)) {
	return token
}

run :: proc(selector: int, result: ^int) {
	switch {
	case selector == 1:
		begin(result, 1)
	case selector == 2:
		begin(result, 2)
	}
	result^ = result^*10 + 3
}

main :: proc() {
	result := 0
	run(1, &result)
	assert(result == 13)
	result = 0
	run(2, &result)
	assert(result == 23)
	result = 0
	run(3, &result)
	assert(result == 3)
}
