#+when DEFERRED_WHEN_ACTIVE
package test_issues

import "core:fmt"

active_test :: proc() {
	fmt.println("active")
}
