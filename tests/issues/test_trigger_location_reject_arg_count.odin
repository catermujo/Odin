package test_trigger_location_reject_arg_count

main :: proc() {
	#panic("too many", #trigger_location, #trigger_location)
}
