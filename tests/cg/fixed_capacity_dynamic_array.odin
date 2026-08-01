package main

Flag :: enum u8 { active }
Flags :: bit_set[Flag; u8]

Entry :: struct {
	flags: Flags,
}

Container :: struct {
	entries: [dynamic; 4]Entry,
}

has_active :: proc(container: ^Container, index: int) -> bool {
	return .active in container.entries[index].flags
}

entry_count :: proc(container: ^Container) -> int {
	return len(container.entries)
}

main :: proc() {
	container: Container
	_ = has_active(&container, 0)
	_ = entry_count(&container)
}
