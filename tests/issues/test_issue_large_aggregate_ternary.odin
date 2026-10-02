// The wasm backend must not receive a first-class PHI for a large aggregate
// produced by a ternary-if expression.

package test_issues

LARGE_AGGREGATE_ELEMENTS :: #config(LARGE_AGGREGATE_ELEMENTS, 1024)

Large_Element :: struct #packed {
	words: [48]u32,
}

Large_Operation :: struct {
	kind:  u8,
	index: u32,
}

Large_Group :: struct {
	count:      int,
	operations: [1024]Large_Operation,
}

Large_Aggregate :: struct {
	vertices: [LARGE_AGGREGATE_ELEMENTS]Large_Element,
	groups:   [16]Large_Group,
}

Large_Container :: struct {
	value: ^Large_Aggregate,
}

copy_aggregate :: proc(container: ^Large_Container, source: ^Large_Aggregate, condition: bool) {
	if container.value != nil {
		container.value^ = {} if condition else source^
	}
}

consume_aggregate :: proc(value: Large_Aggregate) {
	_ = value
}

main :: proc() {
	container: Large_Container
	source: Large_Aggregate
	copy_aggregate(&container, &source, true)
	consume_aggregate({} if container.value == nil else source)
}
