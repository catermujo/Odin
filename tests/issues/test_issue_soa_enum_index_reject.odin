#+test
package test_issues

import "core:testing"

@(test)
test_soa_rejects_other_enum_index :: proc(t: ^testing.T) {
	Axis :: enum {X, Y}
	Other :: enum {X, Y}
	Record :: struct {value: int}
	m: #soa[Axis]Record
	wrong: Other = .X
	_ = m[wrong]
}
