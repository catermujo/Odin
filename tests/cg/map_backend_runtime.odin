package main

main :: proc() {
	string_values := make(map[string]int)
	string_values["entry"] = 7
	if string_values["entry"] != 7 || len(string_values) != 1 {
		panic("CG map string runtime failure")
	}

	int_values := make(map[int]int)
	int_values[3] = 9
	if int_values[3] != 9 || len(int_values) != 1 {
		panic("CG map integer runtime failure")
	}
}
