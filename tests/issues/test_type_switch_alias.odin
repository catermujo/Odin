// Tests duplicate type-switch cases through aliases.
package test_issues

Alias :: int

main :: proc() {
	value: any = 1
	switch _ in value {
	case int:
	case Alias:
	case:
	}
}
