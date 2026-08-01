package main

import "base:intrinsics"

seed :: proc "contextless" (value: uintptr) -> uintptr {
	return value
}

result: uintptr

main :: proc() {
	result = intrinsics.valgrind_client_request(seed(7), seed(1), seed(2), seed(3), seed(4), seed(5), seed(6))
}
