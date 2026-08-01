package main

Item :: struct {
	flags: u8,
}

update_items :: proc(items: ^[dynamic]Item, active: bool) {
	for &item in items^ {
		if active {
			item.flags += 1
		} else {
			item.flags -= 1
		}
	}
}

main :: proc() {
	items: [dynamic]Item
	update_items(&items, true)
}
