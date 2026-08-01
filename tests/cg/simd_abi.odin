package main

Vec :: #simd[4]u32

round_trip :: proc(value: Vec) -> Vec {
	copy := value
	return copy
}

main :: proc() {
	vector := Vec{3, 5, 7, 11}
	vector = round_trip(vector)
}
