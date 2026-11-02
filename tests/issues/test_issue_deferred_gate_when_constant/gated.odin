#+when !ODIN_TEST_DEFERRED_GATE_OFF
package test_issue_deferred_gate_when_constant

import "core:math"

GATED_ENABLE :: math.PI > 3 && #config(ODIN_TEST_DEFERRED_GATE_ENABLE, true)
