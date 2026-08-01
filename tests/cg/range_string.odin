package main

sum_forward :: proc(text: string) -> int {
	result := 0
	for rune, offset in text {
		result += int(rune) + offset
	}
	return result
}

sum_reverse :: proc(text: string) -> int {
	result := 0
	#reverse for rune, offset in text {
		result += int(rune) + offset
	}
	return result
}

main :: proc() {
	_ = sum_forward("aé")
	_ = sum_reverse("aé")
}
