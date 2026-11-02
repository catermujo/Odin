package test_issue_when_constant_over_define

ODIN_TEST_WHEN_GATE_OFF :: true
ODIN_TEST_WHEN_GATE :: #config(ODIN_TEST_WHEN_GATE, false) && !ODIN_TEST_WHEN_GATE_OFF

#assert(define_only_value == 1)
