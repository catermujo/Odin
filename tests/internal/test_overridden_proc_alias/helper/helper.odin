package test_overridden_proc_alias_helper

destroy :: proc(value: ^int) {
	if value != nil {
		value^ = 0
	}
}
