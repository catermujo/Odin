package main

import "core:mem"
import "core:testing"

Entry :: struct {
    memory:    rawptr,
    size:      int,
    alignment: int,
    marker:    u64,
}

expect_entry :: proc(entry: ^Entry, key: rawptr, size: int, marker: u64) {
    assert(entry != nil)
    assert(entry.memory == key)
    assert(entry.size == size)
    assert(entry.alignment == 8)
    assert(entry.marker == marker)
}

make_test_bytes :: proc() -> (data: []byte, err: mem.Allocator_Error) {
    return make([]byte, 32), nil
}

set_test_context_allocator :: proc(allocator: mem.Allocator) {
    context.allocator = allocator
}

test_context_allocator_scope :: proc() {
    previous_allocator := context.allocator
    stack: mem.Rollback_Stack
    assert(mem.rollback_stack_init(&stack, 1024 * 1024) == nil)
    defer mem.rollback_stack_destroy(&stack)

    set_test_context_allocator(mem.rollback_stack_allocator(&stack))
    assert(context.allocator.procedure == previous_allocator.procedure)
    assert(context.allocator.data == previous_allocator.data)
}

test_map_raw_data_key :: proc() -> bool {
    values: map[rawptr]mem.Tracking_Allocator_Entry
    backing, err := make_test_bytes()
    if err != nil {
        return false
    }
    key := raw_data(backing)
    values[key] = mem.Tracking_Allocator_Entry {
        memory    = key,
        size      = 32,
        mode      = .Alloc,
        alignment = 8,
    }

    assert(len(values) == 1)
    entry, ok := &values[key]
    assert(ok)
    assert(entry != nil)
    assert(entry.memory == key)
    assert(entry.size == 32)
    assert(entry.alignment == 8)

    delete(values)
    delete(backing)
    return err == nil
}

test_map_tracking_context :: proc() {
    tracker: mem.Tracking_Allocator
    mem.tracking_allocator_init(&tracker, context.allocator)
    defer mem.tracking_allocator_destroy(&tracker)

    previous_allocator := context.allocator
    context.allocator = mem.tracking_allocator(&tracker)
    defer context.allocator = previous_allocator

    blocks: [10]^[16]u8
    for i in 0 ..< len(blocks) {
        blocks[i] = new([16]u8)
        assert(blocks[i] != nil)
        assert(len(tracker.allocation_map) == i + 1)
    }
    for block in blocks {
        free(block)
    }
    assert(len(tracker.allocation_map) == 0)
}

test_map_tracking_array :: proc() {
    trackers, err := make([]mem.Tracking_Allocator, 12)
    assert(err == nil)
    defer delete(trackers)

    for i in 0 ..< len(trackers) {
        assert(len(trackers[i].allocation_map) == 0)
        mem.tracking_allocator_init(&trackers[i], context.allocator)

        block, alloc_err := mem.alloc(16, 8, mem.tracking_allocator(&trackers[i]))
        assert(alloc_err == nil)
        assert(block != nil)
        assert(len(trackers[i].allocation_map) == 1)
        entry, ok := &trackers[i].allocation_map[block]
        assert(ok)
        assert(entry != nil)
        assert(entry.memory == block)
        assert(entry.size == 16)

        assert(mem.free(block, mem.tracking_allocator(&trackers[i])) == nil)
        assert(len(trackers[i].allocation_map) == 0)
        mem.tracking_allocator_destroy(&trackers[i])
    }
}

test_map_raw_pointer_selector_proc :: proc(allocator_data: rawptr, result: []byte) {
    data := (^mem.Tracking_Allocator)(allocator_data)
    key := raw_data(result)
    data.allocation_map.allocator = context.allocator
    data.allocation_map[key] = mem.Tracking_Allocator_Entry {
        memory    = key,
        size      = len(result),
        mode      = .Alloc,
        alignment = 8,
    }
    assert(len(data.allocation_map) == 1)
    entry, ok := &data.allocation_map[key]
    assert(ok)
    assert(entry != nil)
    assert(entry.memory == key)
    assert(entry.size == len(result))
    assert(entry.alignment == 8)
    delete(data.allocation_map)
}

test_map_raw_pointer_selector :: proc() {
    tracker: mem.Tracking_Allocator
    backing := make([]byte, 32)
    test_map_raw_pointer_selector_proc(rawptr(&tracker), backing)
    delete(backing)
}

test_map_tracking_rollback :: proc() {
    stack: mem.Rollback_Stack
    assert(mem.rollback_stack_init(&stack, 1024 * 1024) == nil)
    defer mem.rollback_stack_destroy(&stack)

    tracker: mem.Tracking_Allocator
    backing := mem.rollback_stack_allocator(&stack)
    mem.tracking_allocator_init(&tracker, backing)
    defer mem.tracking_allocator_destroy(&tracker)
    tracked_allocator := mem.tracking_allocator(&tracker)

    blocks: [10]rawptr
    for i in 0 ..< len(blocks) {
        block, alloc_err := mem.alloc(16 + i, 8, tracked_allocator)
        assert(alloc_err == nil)
        assert(block != nil)
        blocks[i] = block
        assert(len(tracker.allocation_map) == i + 1)
    }
    for block in blocks {
        assert(mem.free(block, tracked_allocator) == nil)
    }
    assert(len(tracker.allocation_map) == 0)
}

@(test)
test_map_insert_address :: proc(t: ^testing.T) {
    _ = t
    keys: [11]u8
    values: map[rawptr]Entry

    for i in 0 ..< 10 {
        key := rawptr(&keys[i])
        marker := u64(i + 100)
        values[key] = Entry {
            memory    = key,
            size      = i + 1,
            alignment = 8,
            marker    = marker,
        }
        assert(len(values) == i + 1)

        if i == 1 {
            inserted := map_insert(&values, key, Entry{memory = key, size = i + 1, alignment = 8, marker = marker})
            assert(inserted != nil)
            assert(len(values) == i + 1)
        }
    }

    for i in 0 ..< 10 {
        key := rawptr(&keys[i])
        marker := u64(i + 100)

        single := &values[key]
        expect_entry(single, key, i + 1, marker)

        pair, ok := &values[key]
        assert(ok)
        expect_entry(pair, key, i + 1, marker)
    }

    missing_key := rawptr(&keys[10])
    assert(&values[missing_key] == nil)
    missing, ok := &values[missing_key]
    assert(missing == nil)
    assert(!ok)

    assert(test_map_raw_data_key())
    test_context_allocator_scope()
    test_map_tracking_context()
    test_map_tracking_array()
    test_map_raw_pointer_selector()
    test_map_tracking_rollback()

    scalars: map[int]int
    for i in 0 ..< 10 {
        scalars[i] = i + 7
    }
    assert(len(scalars) == 10)
    for i in 0 ..< 10 {
        assert(scalars[i] == i + 7)
    }

    tracker: mem.Tracking_Allocator
    mem.tracking_allocator_init(&tracker, context.allocator)
    defer mem.tracking_allocator_destroy(&tracker)
    tracked_allocator := mem.tracking_allocator(&tracker)
    tracked_blocks: [10]rawptr
    for i in 0 ..< len(tracked_blocks) {
        block, alloc_err := mem.alloc(16 + i, 8, tracked_allocator)
        assert(alloc_err == nil)
        assert(block != nil)
        tracked_blocks[i] = block
        assert(len(tracker.allocation_map) == i + 1)
    }
    for i in 0 ..< len(tracked_blocks) {
        block := tracked_blocks[i]
        tracked_entry, tracked_ok := &tracker.allocation_map[block]
        assert(tracked_ok)
        assert(tracked_entry != nil)
        assert(tracked_entry.memory == block)
        assert(tracked_entry.size == 16 + i)
    }
    for i in 0 ..< len(tracked_blocks) {
        block := tracked_blocks[i]
        assert(mem.free(block, tracked_allocator) == nil)
        assert(len(tracker.allocation_map) == len(tracked_blocks) - i - 1)
    }
    assert(len(tracker.allocation_map) == 0)

    delete(values)
    delete(scalars)
}

main :: proc() {  }
