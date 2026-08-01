package main

import "core:testing"

import bt "studio:rt/ds/btree"
import "studio:rt/ds/tic"

Handle :: struct {
    idx: u32,
    gen: u32,
}

Tree :: struct {
    root: Handle,
}

Axis :: enum u8 {
    x,
    y,
}

Value :: union {
    Handle,
    u32,
}

compare_handle :: #force_no_inline proc(left, right: Handle) -> bool {
    return left == right
}

make_handle :: #force_no_inline proc(idx, gen: u32) -> Handle {
    return Handle{idx = idx, gen = gen}
}

test_handle_equality :: proc() {
    zero: Handle
    equal: Handle
    one := Handle {
        idx = 1,
        gen = 2,
    }
    idx_mismatch := Handle {
        idx = 3,
        gen = 2,
    }
    gen_mismatch := Handle {
        idx = 1,
        gen = 4,
    }

    assert(compare_handle(zero, equal))
    assert(!compare_handle(zero, one))
    assert(!compare_handle(one, idx_mismatch))
    assert(!compare_handle(one, gen_mismatch))

    assert(zero == Handle{})
    assert(zero != one)
    assert(make_handle(1, 2) == one)
    assert(make_handle(1, 2) != idx_mismatch)
    assert(make_handle(1, 2) != gen_mismatch)

    left_tree := Tree {
        root = one,
    }
    right_tree := Tree {
        root = one,
    }
    different_tree := Tree {
        root = gen_mismatch,
    }
    assert(left_tree.root == right_tree.root)
    assert(left_tree == right_tree)
    assert(left_tree != different_tree)

    left_ptr := &left_tree.root
    right_ptr := &right_tree.root
    assert(left_ptr^ == right_ptr^)
}

test_aggregate_variants :: proc() {
    left: Value = Handle {
        idx = 5,
        gen = 9,
    }
    right: Value = Handle {
        idx = 5,
        gen = 9,
    }
    different: Value = Handle {
        idx = 5,
        gen = 10,
    }
    other_variant: Value = u32(5)
    assert(left == right)
    assert(left != different)
    assert(left != other_variant)

    left_array := [Axis]Handle {
        .x = Handle{idx = 1, gen = 2},
        .y = Handle{idx = 3, gen = 4},
    }
    right_array := [Axis]Handle {
        .x = Handle{idx = 1, gen = 2},
        .y = Handle{idx = 3, gen = 4},
    }
    different_array := [Axis]Handle {
        .x = Handle{idx = 1, gen = 2},
        .y = Handle{idx = 3, gen = 5},
    }
    assert(left_array == right_array)
    assert(left_array != different_array)
}

test_btree_empty_insert :: proc() {
    tree: bt.BTree(tic.Stamp, u32)
    bt.init(&tree)
    defer bt.destroy(&tree)

    _, existed := bt.set(&tree, max(tic.Stamp), 7)
    assert(!existed)
    assert(bt.len(&tree) == 1)

    value, ok := bt.get(&tree, max(tic.Stamp))
    assert(ok)
    assert(value == 7)
}

@(test)
test_aggregate_equality :: proc(t: ^testing.T) {
    _ = t
    test_handle_equality()
    test_aggregate_variants()
    test_btree_empty_insert()
}

main :: proc() {  }
