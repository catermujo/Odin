package test_trigger_location_use_site

Bad :: struct {
	x: int,
	handle: int,
}

trigger :: proc($T: typeid) {
	#assert(offset_of(T, handle) == 0)
}

main :: proc() {
	trigger(Bad)
}
