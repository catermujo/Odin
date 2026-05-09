// Minimal repro: array-programming on a heap-allocated large fixed array can blow stack.
// Run: ./odin run tests/issues/repro_array_programming_stack_overflow.odin -file -debug
package main

import "base:runtime"
import "core:fmt"

N :: 2 * runtime.Megabyte
M :: N + 3

main :: proc() {
	a := new([N]f32)
	b := new([N]f32)
	c := new([M]f32)
	d := new([M]f32)

	// These lines used to materialize giant stack temporaries.
	a^ += 10
	b^ = b^ + 10
	c^ += 10
	d^ = d^ + 10

	fmt.println(a[0], a[len(a)-1], len(a))
	fmt.println(b[0], b[len(b)-1], len(b))
	fmt.println(c[0], c[len(c)-1], len(c))
	fmt.println(d[0], d[len(d)-1], len(d))
}
