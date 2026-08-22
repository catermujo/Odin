// An implementation of Myers' O(ND) Difference Algorithm for slices of
// arbitrary types that can be simply compared.
//
// See https://publications.mpi-cbg.de/Myers_1986_6330.pdf
package slice

import "base:intrinsics"

import "core:mem"

// A kind of difference in an edit script.
Diff_Kind :: enum {
	// A value should be inserted in the original slice.
	Insert,
	// A value should be deleted from the original slice.
	Delete,
	// A value should be kept in the original slice.
	Keep,
}

// A difference in an edit script.
Diff :: struct($T: typeid) {
	kind:       Diff_Kind,
	begin, end: int,
	values:     []T,
}

/*
Calculates the difference list between a given original list and its expected
sequence of elements as an edit script.

Inputs:
- value: The original slice.
- expected: The expected slice.
- allocator: The allocator used to create the edit script (default is context.allocator).

Returns:
- result: The edit script to transform the original list in the expected list.
- err: An `Allocator_Error`, if allocation failed.
*/
diff :: proc(
	value, expected: $S/[]$T,
	allocator := context.allocator,
) -> (
	result: []Diff(T),
	err: mem.Allocator_Error,
) where intrinsics.type_is_simple_compare(T) #optional_allocator_error {
	a := value
	b := expected

	max := len(a) + len(b)
	// V needs one sentinel slot on either side because each diagonal reads
	// its k-1 and k+1 neighbours.
	offset := max + 1
	v := make([]int, 2 * max + 3, allocator) or_return
	defer delete(v, allocator)
	fill(v, -1)
	v[offset + 1] = 0

	trace := make([dynamic][]int, allocator) or_return
	defer {
		for snapshot in trace {
			delete(snapshot, allocator)
		}
		delete(trace)
	}

	distance := -1
	for d := 0; d <= max; d += 1 {
		reached_end := false

		for k := -d; k <= d; k += 2 {
			// Valid edit-graph diagonals satisfy -len(b) <= k <= len(a).
			if k < -len(b) || k > len(a) {
				continue
			}

			x: int
			if k == -d || (k != d && v[k - 1 + offset] < v[k + 1 + offset]) {
				x = v[k + 1 + offset]
			} else {
				x = v[k - 1 + offset] + 1
			}
			y := x - k

			for x >= 0 && y >= 0 && x < len(a) && y < len(b) && a[x] == b[y] {
				x += 1
				y += 1
			}
			v[k + offset] = x

			if x >= len(a) && y >= len(b) {
				reached_end = true
				break
			}
		}

		snapshot := make([]int, len(v), allocator) or_return
		copy(snapshot, v)
		_, append_err := append(&trace, snapshot)
		if append_err != nil {
			delete(snapshot, allocator)
			return nil, append_err
		}

		if reached_end {
			distance = d
			break
		}
	}

	assert(distance >= 0)

	edits := make([dynamic]Edit, allocator)
	defer delete(edits)

	x, y := len(a), len(b)
	for d := distance; d > 0; d -= 1 {
		previous := trace[d - 1]
		k := x - y

		previous_k: int
		if k == -d || (k != d && previous[k - 1 + offset] < previous[k + 1 + offset]) {
			previous_k = k + 1
		} else {
			previous_k = k - 1
		}

		previous_x := previous[previous_k + offset]
		previous_y := previous_x - previous_k

		for x > previous_x && y > previous_y {
			append(&edits, Edit{kind = .Keep, position = x - 1}) or_return
			x -= 1
			y -= 1
		}

		if x == previous_x {
			append(&edits, Edit{kind = .Insert, position = y - 1}) or_return
			y -= 1
		} else {
			append(&edits, Edit{kind = .Delete, position = x - 1}) or_return
			x -= 1
		}
	}

	for x > 0 && y > 0 {
		append(&edits, Edit{kind = .Keep, position = x - 1}) or_return
		x -= 1
		y -= 1
	}
	for x > 0 {
		append(&edits, Edit{kind = .Delete, position = x - 1}) or_return
		x -= 1
	}
	for y > 0 {
		append(&edits, Edit{kind = .Insert, position = y - 1}) or_return
		y -= 1
	}

	for i := 0; i < len(edits) / 2; i += 1 {
		j := len(edits) - i - 1
		edits[i], edits[j] = edits[j], edits[i]
	}

	script := make([dynamic]Diff(T), allocator)
	defer if err != nil {delete(script)}
	for edit in edits {
		append_diff(&script, a, b, edit.kind, edit.position) or_return
	}

	return script[:], nil

	Edit :: struct {
		kind:     Diff_Kind,
		position: int,
	}

	append_diff :: proc(
		diffs: ^[dynamic]Diff(T),
		a, b: S,
		kind: Diff_Kind,
		position: int,
	) -> (
		err: mem.Allocator_Error,
	) {
		if len(diffs) > 0 {
			last := &diffs[len(diffs) - 1]

			if last.kind == kind && last.end == position {
				last.end = position + 1

				switch last.kind {
				case .Insert:
					last.values = b[last.begin:last.end]
				case .Delete, .Keep:
					last.values = a[last.begin:last.end]
				}

				return nil
			}
		}

		values: S
		switch kind {
		case .Insert:
			values = b[position:position + 1]
		case .Delete, .Keep:
			values = a[position:position + 1]
		}

		append(
			diffs,
			Diff(T){kind = kind, begin = position, end = position + 1, values = values},
		) or_return

		return nil
	}
}
