package main

classify :: proc(r: rune) -> int {
	switch r {
	case '0'..='9':
		return 1
	case 'a'..<'z':
		return 2
	case 'Z', '_':
		return 3
	case:
		return -1
	}
}

Version :: enum u32 {
	First = 12,
	Middle = 40,
	Last = 124,
}

classify_version :: proc(version: Version) -> int {
	switch version {
	case .First..=.Last:
		return 1
	case:
		return 0
	}
}

main :: proc() {
	_ = classify('m')
	_ = classify_version(.Middle)
}
