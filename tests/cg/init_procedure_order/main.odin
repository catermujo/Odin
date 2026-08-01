package main

import zdep "init_order:zdep"

observed: bool

@(init)
main_init :: proc "contextless" () {
	observed = zdep.ready
}

main :: proc() {
	assert(observed)
}
