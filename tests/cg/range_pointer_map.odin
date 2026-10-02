package main

Map :: map[string]int

iterate :: proc(m: ^Map) {
	for key, &value in m {
		_ = key
		_ = value
	}
}

main :: proc() {
	m: Map
	iterate(&m)
}
