#+build linux
#+test
package tests_core_posix

import "core:sys/posix"
import "core:testing"

// DUMBAI: A successful child must stay successful when a watchdog forwards its termination status.
@(test)
test_linux_wait_signal_status :: proc(t: ^testing.T) {
	cases := [?]struct {
		status:   i32,
		signaled: bool,
	}{
		{0, false},
		{1 << 8, false},
		{255 << 8, false},
		{i32(posix.Signal.SIGTERM), true},
		{i32(posix.Signal.SIGSEGV), true},
		{i32(posix.Signal.SIGSEGV) | 0x80, true},
		{0x7f, false},
		{(i32(posix.Signal.SIGSTOP) << 8) | 0x7f, false},
		{0xffff, false},
	}
	for test in cases {
		testing.expect_value(t, posix.WIFSIGNALED(test.status), test.signaled)
	}
}
