#+feature dynamic-literals
package test_core_slice

import "core:slice"
import "core:testing"
import "core:math/rand"
import "core:log"
import "core:mem"

@test
test_sort_with_indices :: proc(t: ^testing.T) {
	// Test sizes are all prime.
	test_sizes :: []int{7, 13, 347, 1031, 10111, 100003}

	for test_size in test_sizes {
		rand.reset(t.seed)

		vals  := make([]u64, test_size)
		r_idx := make([]int, test_size) // Reverse index
		defer {
			delete(vals)
			delete(r_idx)
		}

		// Set up test values
		for _, i in vals {
			vals[i] = rand.uint64()
		}

		// Sort
		f_idx := slice.sort_with_indices(vals)
		defer delete(f_idx)

		// Verify sorted test values
		rand.reset(t.seed)

		for v, i in f_idx {
			r_idx[v] = i
		}

		last: u64
		for v, i in vals {
			if i > 0 {
				val_pass := v >= last
				testing.expect(t, val_pass, "Expected randomized test values to have been sorted")
				if !val_pass {
					break
				}
			}

			idx_pass := vals[r_idx[i]] == rand.uint64()
			testing.expect(t, idx_pass, "Expected index to have been sorted")
			if !idx_pass {
				break
			}
			last = v
		}
	}
}

@test
test_sort_by_indices :: proc(t: ^testing.T) {
	// Test sizes are all prime.
	test_sizes :: []int{7, 13, 347, 1031, 10111, 100003}

	for test_size in test_sizes {
		rand.reset(t.seed)

		vals  := make([]u64, test_size)
		r_idx := make([]int, test_size) // Reverse index
		defer {
			delete(vals)
			delete(r_idx)
		}

		// Set up test values
		for _, i in vals {
			vals[i] = rand.uint64()
		}

		// Sort
		f_idx := slice.sort_with_indices(vals)
		defer delete(f_idx)

		// Verify sorted test values
		rand.reset(t.seed)

		{
			indices := make([]int, test_size)
			defer delete(indices)
			for _, i in indices {
				indices[i] = i
			}

			sorted_indices := slice.sort_by_indices(indices, f_idx)
			defer delete(sorted_indices)
			for v, i in sorted_indices {
				idx_pass := v == f_idx[i]
				testing.expect(t, idx_pass, "Expected the sorted index to be the same as the result from sort_with_indices")
				if !idx_pass {
					break
				}
			}
		}
		{
			indices := make([]int, test_size)
			defer delete(indices)
			for _, i in indices {
				indices[i] = i
			}

			slice.sort_by_indices_overwrite(indices, f_idx)
			for v, i in indices {
				idx_pass := v == f_idx[i]
				testing.expect(t, idx_pass, "Expected the sorted index to be the same as the result from sort_with_indices")
				if !idx_pass {
					break
				}
			}
		}
		{
			indices := make([]int, test_size)
			swap := make([]int, test_size)
			defer {
				delete(indices)
				delete(swap)
			}
			for _, i in indices {
				indices[i] = i
			}

			slice.sort_by_indices(indices, swap, f_idx)
			for v, i in swap {
				idx_pass := v == f_idx[i]
				testing.expect(t, idx_pass, "Expected the sorted index to be the same as the result from sort_with_indices")
				if !idx_pass {
					break
				}
			}
		}
	}
}

@test
test_sort_stability :: proc(t: ^testing.T) {
	// Test sizes are all prime.
	test_sizes :: []int{7, 13, 347, 1031, 10111, 100003}
	Data :: struct {
		rand: int,
		index: int,
	}

	for test_size in test_sizes {
		rand.reset(t.seed)

		vals  := make([]Data, test_size)
		defer delete(vals)

		// Set up test values
		for &val, i in vals {
			val = {rand.int_max(10), i}
		}

		// Sort
		slice.stable_sort_by(vals, proc(l, r: Data) -> bool {return l.rand < r.rand})

		// Verify sorted test values
		rand.reset(t.seed)

		sum := vals[0].index
		for i in 1..<len(vals) {
			sum += vals[i].index
			if vals[i - 1].rand > vals[i].rand {
				testing.expect(t, false, "Expected slice to be sorted")
			}
			if vals[i - 1].rand < vals[i].rand {
				continue
			}
			if vals[i - 1].index > vals[i].index {
				testing.expect(t, false, "Expected slice to be stable")
			}
		}

		testing.expect(t, sum == test_size * (test_size - 1) / 2, "Expected slice to have all indecies")

	}
}


@test
test_binary_search :: proc(t: ^testing.T) {
	index: int
	found: bool

	s := []i32{0, 1, 1, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55}

	index, found = slice.binary_search(s, 13)
	testing.expect(t, index == 9,    "Expected index to be 9")
	testing.expect(t, found == true, "Expected found to be true")

	index, found = slice.binary_search(s, 4)
	testing.expect(t, index == 7,     "Expected index to be 7.")
	testing.expect(t, found == false, "Expected found to be false.")

	index, found = slice.binary_search(s, 100)
	testing.expect(t, index == 13,    "Expected index to be 13.")
	testing.expect(t, found == false, "Expected found to be false.")

	index, found = slice.binary_search(s, 1)
	testing.expect(t, index >= 1 && index <= 4, "Expected index to be 1, 2, 3, or 4.")
	testing.expect(t, found == true, "Expected found to be true.")

	index, found = slice.binary_search(s, -1)
	testing.expect(t, index == 0,     "Expected index to be 0.")
	testing.expect(t, found == false, "Expected found to be false.")

	a := []i32{}

	index, found = slice.binary_search(a, 13)
	testing.expect(t, index == 0,     "Expected index to be 0.")
	testing.expect(t, found == false, "Expected found to be false.")

	b := []i32{1}

	index, found = slice.binary_search(b, 13)
	testing.expect(t, index == 1,     "Expected index to be 1.")
	testing.expect(t, found == false, "Expected found to be false.")

	index, found = slice.binary_search(b, 1)
	testing.expect(t, index == 0,    "Expected index to be 0.")
	testing.expect(t, found == true, "Expected found to be true.")

	index, found = slice.binary_search(b, 0)
	testing.expect(t, index == 0,     "Expected index to be 0.")
	testing.expect(t, found == false, "Expected found to be false.")
}

@test
test_permutation_iterator :: proc(t: ^testing.T) {
	// Big enough to do some sanity checking but not overly large.
	FAC_5 :: 120
	s := []int{1, 2, 3, 4, 5}
	seen: map[int]bool
	defer delete(seen)

	iter := slice.make_permutation_iterator(s)
	defer slice.destroy_permutation_iterator(iter)

	permutations_counted: int
	for slice.permute(&iter) {
		n := 0
		for item in s {
			n *= 10
			n += item
		}
		if n in seen {
			log.error("Permutation iterator made a duplicate permutation.")
			return
		}
		seen[n] = true
		permutations_counted += 1
	}

	testing.expect_value(t, len(seen), FAC_5)
	testing.expect_value(t, permutations_counted, FAC_5)
}

// Test inputs from #3276 and #3769
UNIQUE_TEST_VECTORS :: [][2][]int{
	{{2,2,2},             {2}},
	{{1,1,1,2,2,3,3,3,3}, {1,2,3}},
	{{1,2,4,4,5},         {1,2,4,5}},
}

@test
test_compare_empty :: proc(t: ^testing.T) {
	a := []int{}
	b := []int{}
	c: [dynamic]int = { 0 }
	d: [dynamic]int = { 1 }
	clear(&c)
	clear(&d)
	defer {
		delete(c)
		delete(d)
	}

	testing.expectf(t, len(a) == 0,
		"Expected length of slice `a` to be zero")
	testing.expectf(t, len(c) == 0,
		"Expected length of dynamic array `c` to be zero")
	testing.expectf(t, len(d) == 0,
		"Expected length of dynamic array `d` to be zero")

	testing.expectf(t, slice.equal(a, a),
		"Expected empty slice to be equal to itself")
	testing.expectf(t, slice.equal(a, b),
		"Expected two different but empty stack-based slices to be equivalent")
	testing.expectf(t, slice.equal(a, c[:]),
		"Expected empty slice to be equal to slice of empty dynamic array")
	testing.expectf(t, slice.equal(c[:], d[:]),
		"Expected two separate empty slices of two dynamic arrays to be equal")
}

@test
test_linear_search_reverse :: proc(t: ^testing.T) {
	index: int
	found: bool

	s := []i32{0, 50, 50, 100}

	index, found = slice.linear_search_reverse(s, 100)
	testing.expect(t, found)
	testing.expect_value(t, index, len(s) - 1)

	index, found = slice.linear_search_reverse(s[len(s) - 1:], 100)
	testing.expect(t, found)
	testing.expect_value(t, index, 0)

	index, found = slice.linear_search_reverse(s, 50)
	testing.expect(t, found)
	testing.expect_value(t, index, 2)

	index, found = slice.linear_search_reverse(s, 0)
	testing.expect(t, found)
	testing.expect_value(t, index, 0)

	index, found = slice.linear_search_reverse(s, -1)
	testing.expect(t, !found)

	less_than_80 :: proc(x: i32) -> bool {
		return x < 80
	}

	index, found = slice.linear_search_reverse_proc(s, less_than_80)
	testing.expect(t, found)
	testing.expect_value(t, index, 2)
}

@test
test_diff :: proc(t: ^testing.T) {
	script: []slice.Diff(rune)
	err: mem.Allocator_Error

	{
		a := []rune{'A', 'B', 'C', 'A', 'B', 'B', 'A'}
		b := []rune{'C', 'B', 'A', 'B', 'A', 'C'}
		script, err = slice.diff(a, b)
		defer delete(script)

		testing.expect_value(t, err, nil)
		if !testing.expect_value(t, len(script), 7) {return}
		ai, bi, cost := 0, 0, 0
		for edit in script {
			n := len(edit.values)
			switch edit.kind {
			case .Keep:
				testing.expect(t, ai + n <= len(a) && bi + n <= len(b))
				testing.expect(t, slice.equal(edit.values, a[ai:ai + n]))
				testing.expect(t, slice.equal(edit.values, b[bi:bi + n]))
				ai += n
				bi += n
			case .Delete:
				testing.expect(t, ai + n <= len(a))
				testing.expect(t, slice.equal(edit.values, a[ai:ai + n]))
				ai += n
				cost += n
			case .Insert:
				testing.expect(t, bi + n <= len(b))
				testing.expect(t, slice.equal(edit.values, b[bi:bi + n]))
				bi += n
				cost += n
			}
		}
		testing.expect_value(t, ai, len(a))
		testing.expect_value(t, bi, len(b))
		testing.expect_value(t, cost, 5)
	}

	{
		script, err = slice.diff([]rune{0}, []rune{0, 0})
		defer delete(script)

		testing.expect_value(t, err, nil)
		if !testing.expect_value(t, len(script), 2) {return}
		testing.expect_value(t, script[0].kind, slice.Diff_Kind.Keep)
		testing.expect_value(t, script[0].begin, 0)
		testing.expect_value(t, script[0].end, 1)
		testing.expect_value(t, script[0].values[0], rune(0))
		testing.expect_value(t, script[1].kind, slice.Diff_Kind.Insert)
		testing.expect_value(t, script[1].begin, 1)
		testing.expect_value(t, script[1].end, 2)
		testing.expect_value(t, script[1].values[0], rune(0))
	}

	{
		int_script: []slice.Diff(int)
		int_script, err = slice.diff(
			[]int{0, 0, 1, 1, 0, 0},
			[]int{1, 0, 0, 0, 0, 1},
		)
		defer delete(int_script)

		testing.expect_value(t, err, nil)
		if !testing.expect_value(t, len(int_script), 5) {return}
		testing.expect_value(t, int_script[0].kind, slice.Diff_Kind.Insert)
		testing.expect_value(t, int_script[0].begin, 0)
		testing.expect_value(t, int_script[0].end, 1)
		testing.expect_value(t, int_script[0].values[0], 1)
		testing.expect_value(t, int_script[1].kind, slice.Diff_Kind.Keep)
		testing.expect_value(t, int_script[1].begin, 0)
		testing.expect_value(t, int_script[1].end, 2)
		testing.expect_value(t, int_script[2].kind, slice.Diff_Kind.Delete)
		testing.expect_value(t, int_script[2].begin, 2)
		testing.expect_value(t, int_script[2].end, 4)
		testing.expect_value(t, int_script[3].kind, slice.Diff_Kind.Keep)
		testing.expect_value(t, int_script[3].begin, 4)
		testing.expect_value(t, int_script[3].end, 6)
		testing.expect_value(t, int_script[4].kind, slice.Diff_Kind.Insert)
		testing.expect_value(t, int_script[4].begin, 5)
		testing.expect_value(t, int_script[4].end, 6)
	}

	{
		int_script: []slice.Diff(int)
		int_script, err = slice.diff([]int{0}, []int{1, 1, 1, 1, 1})
		defer delete(int_script)

		testing.expect_value(t, err, nil)
		if !testing.expect_value(t, len(int_script), 2) {return}
		testing.expect_value(t, int_script[0].kind, slice.Diff_Kind.Delete)
		testing.expect_value(t, int_script[0].begin, 0)
		testing.expect_value(t, int_script[0].end, 1)
		testing.expect_value(t, int_script[0].values[0], 0)
		testing.expect_value(t, int_script[1].kind, slice.Diff_Kind.Insert)
		testing.expect_value(t, int_script[1].begin, 0)
		testing.expect_value(t, int_script[1].end, 5)
		if !testing.expect_value(t, len(int_script[1].values), 5) {return}
		testing.expect_value(t, int_script[1].values[0], 1)
		testing.expect_value(t, int_script[1].values[4], 1)
	}

	{
		script, err = slice.diff([]rune{}, []rune{'A', 'B', 'C'})
		defer delete(script)

		testing.expect_value(t, err, nil)
		if !testing.expect_value(t, len(script), 1) {return}
		testing.expect_value(t, script[0].kind, slice.Diff_Kind.Insert)
		testing.expect_value(t, script[0].begin, 0)
		testing.expect_value(t, script[0].end, 3)
		if !testing.expect_value(t, len(script[0].values), 3) {return}
		testing.expect_value(t, script[0].values[0], 'A')
		testing.expect_value(t, script[0].values[1], 'B')
		testing.expect_value(t, script[0].values[2], 'C')
	}

	{
		script, err = slice.diff([]rune{'A', 'B', 'C'}, []rune{})
		defer delete(script)

		testing.expect_value(t, err, nil)
		if !testing.expect_value(t, len(script), 1) {return}
		testing.expect_value(t, script[0].kind, slice.Diff_Kind.Delete)
		testing.expect_value(t, script[0].begin, 0)
		testing.expect_value(t, script[0].end, 3)
		if !testing.expect_value(t, len(script[0].values), 3) {return}
		testing.expect_value(t, script[0].values[0], 'A')
		testing.expect_value(t, script[0].values[1], 'B')
		testing.expect_value(t, script[0].values[2], 'C')
	}

	{
		script, err = slice.diff([]rune{'O', 'd', 'i', 'n'}, []rune{'O', 'd', 'i', 'n'})
		defer delete(script)

		testing.expect_value(t, err, nil)
		if !testing.expect_value(t, len(script), 1) {return}
		testing.expect_value(t, script[0].kind, slice.Diff_Kind.Keep)
		testing.expect_value(t, script[0].begin, 0)
		testing.expect_value(t, script[0].end, 4)
		if !testing.expect_value(t, len(script[0].values), 4) {return}
		testing.expect_value(t, script[0].values[0], 'O')
		testing.expect_value(t, script[0].values[1], 'd')
		testing.expect_value(t, script[0].values[2], 'i')
		testing.expect_value(t, script[0].values[3], 'n')
	}

	{
		script, err = slice.diff([]rune{}, []rune{})
		defer delete(script)
		testing.expect_value(t, err, nil)
		if !testing.expect_value(t, len(script), 0) {return}
	}

	{
		a := []int{0}
		b := []int{1, 1, 1, 1, 1}
		saw_oom, saw_success: bool

		// Sweep every allocation failure point in this small diff. The tracking
		// allocator proves that error cleanup releases all allocations, including
		// a snapshot whose trace append fails.
		for fail_after in 0..<64 {
			tracking: mem.Tracking_Allocator
			mem.tracking_allocator_init(&tracking, context.allocator)

			failing := Failing_Allocator{
				backing = mem.tracking_allocator(&tracking),
				fail_after = fail_after,
			}
			failing_allocator := failing_allocator(&failing)
			failure_script, failure_err := slice.diff(a, b, failing_allocator)
			delete(failure_script, failing_allocator)

			if failure_err == nil {
				saw_success = true
			} else if failure_err == .Out_Of_Memory {
				saw_oom = true
			}
			testing.expect(t, failure_err == nil || failure_err == .Out_Of_Memory)
			testing.expect_value(t, tracking.current_memory_allocated, i64(0))
			testing.expect_value(t, len(tracking.allocation_map), 0)

			mem.tracking_allocator_destroy(&tracking)
		}

		testing.expect(t, saw_oom)
		testing.expect(t, saw_success)
	}
}

Failing_Allocator :: struct {
	backing:           mem.Allocator,
	fail_after:        int,
	allocation_count: int,
}

failing_allocator :: proc(data: ^Failing_Allocator) -> mem.Allocator {
	return mem.Allocator{
		procedure = failing_allocator_proc,
		data = data,
	}
}

failing_allocator_proc :: proc(
	allocator_data: rawptr,
	mode: mem.Allocator_Mode,
	size, alignment: int,
	old_memory: rawptr,
	old_size: int,
	loc := #caller_location,
) -> ([]byte, mem.Allocator_Error) {
	failing := (^Failing_Allocator)(allocator_data)
	if mode == .Alloc || mode == .Alloc_Non_Zeroed {
		if failing.allocation_count >= failing.fail_after {
			return nil, .Out_Of_Memory
		}
		failing.allocation_count += 1
	}

	return failing.backing.procedure(
		failing.backing.data,
		mode,
		size,
		alignment,
		old_memory,
		old_size,
		loc,
	)
}
