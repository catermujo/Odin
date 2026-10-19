// DUMBAI: A record or map instantiated with the type `typeid` itself is specialized, not polymorphic. The checker
// used to treat it as an unbound `$T: typeid`, never published the record's fields, and hung the build.
package main

Box :: struct($T: typeid) {
	value: T,
}

make_box :: proc($T: typeid) -> Box(T) where size_of(Box(T)) > 0 {
	return {}
}

count_after_delete :: proc(_: ^$T) -> int {
	Registry :: struct {
		seen: map[typeid]int,
	}
	registry: Registry
	visit :: proc(registry: ^Registry, id: typeid) -> int {
		registry.seen[id] = 1
		delete_key(&registry.seen, id)
		return len(registry.seen)
	}
	return visit(&registry, T)
}

main :: proc() {
	box := make_box(typeid)
	assert(box.value == nil)
	value := 1
	assert(count_after_delete(&value) == 0)
}
