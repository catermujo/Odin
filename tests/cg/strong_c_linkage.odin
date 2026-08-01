package main

@(link_name="cg_strong_c_entry", linkage="strong")
strong_c_entry :: proc "c" () {
}

main :: proc() {
	strong_c_entry()
}
