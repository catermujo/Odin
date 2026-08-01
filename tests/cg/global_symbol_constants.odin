package main

seed_i32 :: proc "contextless" () -> i32 {
	return 13
}

global_data: i32

@(export)
procedure_value: proc "contextless" () -> i32 = seed_i32

@(export)
pointer_value: ^i32 = &global_data

foreign {
	foreign_seed :: proc "c" () -> i32 ---
}

@(export)
foreign_procedure_value: proc "c" () -> i32 = foreign_seed

main :: proc() {}
