package test_trigger_location_accept

main :: proc() {
	#assert(true, #trigger_location)
	#assert(true, "still okay", #trigger_location)
	#panic("ignored panic", #trigger_location)
}
