package test_issue_deferred_gate_when_constant

when GATED_ENABLE {
	gated_value :: 1
}

#assert(gated_value == 1)
