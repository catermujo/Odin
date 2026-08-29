package test_issues

import "core:bytes"
import "core:testing"

value :: proc {typed_value, string_value}
typed_value :: proc($T: typeid, n: int, bias: int = 1) -> T {
	return T(n + bias)
}
string_value :: proc(s: string, n: int, bias: int = 1) -> string {
	return s
}

tag :: proc {constant_tag, string_tag}
constant_tag :: proc($K: int, n: int) -> int {
	return K + n
}
string_tag :: proc(s: string, n: int) -> int {
	return n
}

@(test)
proc_group_cached_signature :: proc(t: ^testing.T) {
	n := 3
	first := value(int, n)
	second := value(int, n)
	testing.expect_value(t, first, 4)
	testing.expect_value(t, second, 4)

	bias := 2
	named_first := value(int, n, bias = bias)
	named_second := value(int, n, bias = bias)
	testing.expect_value(t, named_first, 5)
	testing.expect_value(t, named_second, 5)

	K :: int(7)
	tagged_first := tag(K, n)
	tagged_second := tag(K, n)
	testing.expect_value(t, tagged_first, 10)
	testing.expect_value(t, tagged_second, 10)
}

@(test)
bytes_make_cached_signature :: proc(t: ^testing.T) {
	input := []u8{1, 2, 3}
	first, first_err := bytes.clone_safe(input)
	second, second_err := bytes.clone_safe(input)
	defer delete(first)
	defer delete(second)
	testing.expect_value(t, first_err, nil)
	testing.expect_value(t, second_err, nil)
	testing.expect(t, bytes.equal(first, input))
	testing.expect(t, bytes.equal(second, input))
}
