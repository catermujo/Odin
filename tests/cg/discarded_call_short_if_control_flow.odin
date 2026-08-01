package cg_discarded_call_short_if_control_flow

recreate :: proc() -> bool {
	return true
}

ensure_depth :: proc() -> bool {
	return true
}

counter: int

finish :: proc() {
	counter += 1
}

run :: proc() {
	needs_recreate := true
	has_world_pass := true
	defer finish()
	defer finish()

	if needs_recreate {
		width, height := 1, 1
		if width > 0 && height > 0 {
			if recreate() && has_world_pass do _ = ensure_depth()
		}
		return
	}
	counter += 0
}

main :: proc() {
	run()
	assert(counter == 2)
}
