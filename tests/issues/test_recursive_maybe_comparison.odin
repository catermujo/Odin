package test_issues

Node :: struct {
	next: Maybe(Node),
}

main :: proc() {
	node: Node
	_ = node.next == nil
}
