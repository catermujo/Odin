package zero_address_repro

Zero :: struct {}

foreign {
	zero_sink :: proc "c" (value: ^Zero) ---
}

@(export)
witness :: proc "c" () {
	value: Zero
	zero_sink(&value)
}

main :: proc() {}
