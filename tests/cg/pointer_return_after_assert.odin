package main

import "core:container/xar"

Item :: struct {
	value: int,
}

Block :: struct {
	item: Item,
}

HVec2 :: [2]f16
UV_Quad :: [4]HVec2
DEFAULT_UVMAP: UV_Quad : {{0, 0}, {0, 1}, {1, 1}, {1, 0}}

trap :: proc() -> ! {
	panic("unexpected")
}

get_unsafe :: #force_inline proc(items: ^[dynamic]Item) -> ^Item {
	return &items[0]
}

get :: #force_inline proc(items: ^[dynamic]Item) -> ^Item {
	assert(len(items) > 0)
	return get_unsafe(items)
}

get_xar_unsafe :: #force_inline proc "contextless" (items: ^xar.Array(Item, 2)) -> ^Item {
	return xar.array_get_ptr_unsafe(items, 0)
}

get_xar :: #force_inline proc(items: ^xar.Array(Item, 2)) -> ^Item {
	assert(items.len > 0)
	return get_xar_unsafe(items)
}

get_block_xar_unsafe :: #force_inline proc "contextless" (items: ^xar.Array(Block, 2)) -> ^Item {
	return &xar.array_get_ptr_unsafe(items, 0).item
}

valid_block_xar :: #force_no_inline proc(items: ^xar.Array(Block, 2)) -> bool {
	return items.len > 0
}

get_block_xar :: #force_inline proc(items: ^xar.Array(Block, 2), enabled: bool, loc := #caller_location) -> ^Item {
	if enabled && !valid_block_xar(items) {
		trap()
	}
	assert(items.len > 0)
	return get_block_xar_unsafe(items)
}

main :: proc() {
	items := make([dynamic]Item)
	defer delete(items)
	append(&items, Item{42})
	item := get(&items)
	assert(item != nil)
	uvs: UV_Quad = DEFAULT_UVMAP
	assert(uvs[2][0] == 1)
	assert(item.value == 42)

	x: xar.Array(Item, 2)
	xar.array_init(&x)
	defer xar.array_destroy(&x)
	_, err := xar.array_push_back_elem(&x, Item{13})
	assert(err == nil)
	assert(get_xar(&x).value == 13)

	blocks: xar.Array(Block, 2)
	xar.array_init(&blocks)
	defer xar.array_destroy(&blocks)
	_, block_err := xar.array_push_back_elem(&blocks, Block{Item{7}})
	assert(block_err == nil)
	assert(get_block_xar(&blocks, false).value == 7)
}
