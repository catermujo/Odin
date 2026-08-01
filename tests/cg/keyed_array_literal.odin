package main

Point :: struct {
	x, y: f32,
}

Points :: [3]Point

State :: struct {
	points: Points,
}

keyed_scalars :: proc() -> [3]i32 {
	return {0 = 1, 2 = 3}
}

keyed_points :: proc() -> Points {
	return {0 = {x = 1, y = 2}, 2 = {x = 3, y = 4}}
}

keyed_nested :: proc() -> State {
	return {points = {1 = {x = 5, y = 6}}}
}

main :: proc() {
	_ = keyed_scalars()
	_ = keyed_points()
	_ = keyed_nested()
}
