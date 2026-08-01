package cg_source_code_location_package

import "base:runtime"

capture :: proc(loc := #caller_location) -> runtime.Source_Code_Location {
	return loc
}

main :: proc() {
	loc := capture()
	assert(loc.package_name == "cg_source_code_location_package")
}
