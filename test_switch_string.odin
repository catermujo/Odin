// Test: switch on string
//
// Fast-backend (as of the variadic-pack / sret fixes) supports
// `switch` on scalar tags but falls back to LLVM for string-typed tags.
// This test exercises the string-tag path and checks all the routes:
//
//   1. Tag is a local string variable (load address via lea + spill).
//   2. Cases are string literals (materialize as `{data, len}` to a
//      temp, then field-by-field byte compare against the tag).
//   3. Matched case, default case, and no-match (all return values).
//
// The expected output is:
//
//   g_match_first   = 1
//   g_match_middle  = 2
//   g_match_last    = 3
//   g_default       = 99
//   g_no_match      = 0
package test_switch_string

import "core:fmt"

g_match_first:  int
g_match_middle: int
g_match_last:   int
g_default:      int
g_no_match:     int

classify :: proc(s: string) -> int {
	switch s {
	case "apple":
		return 1
	case "banana":
		return 2
	case "cherry":
		return 3
	case:
		return 99
	}
}

// No default — any unmatched tag falls off the end and we return 0.
classify_no_default :: proc(s: string) -> int {
	switch s {
	case "apple":
		return 10
	case "banana":
		return 20
	}
	return 0
}

do_test :: proc() {
	g_match_first  = classify("apple")
	g_match_middle = classify("banana")
	g_match_last   = classify("cherry")
	g_default      = classify("grape")              // hits default → 99
	g_no_match     = classify_no_default("")       // no match, no default → 0
}

main :: proc() {
	do_test()
	fmt.println("g_match_first  =", g_match_first)  // 1
	fmt.println("g_match_middle =", g_match_middle) // 2
	fmt.println("g_match_last   =", g_match_last)   // 3
	fmt.println("g_default      =", g_default)      // 99
	fmt.println("g_no_match     =", g_no_match)     // 0

	if g_match_first  != 1   { fmt.println("FAIL: match_first")  }
	if g_match_middle != 2   { fmt.println("FAIL: match_middle") }
	if g_match_last   != 3   { fmt.println("FAIL: match_last")   }
	if g_default      != 99  { fmt.println("FAIL: default")      }
	if g_no_match     != 0   { fmt.println("FAIL: no_match")     }
}
