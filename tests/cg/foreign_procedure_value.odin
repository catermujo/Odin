package main

Foreign_Proc :: proc "c" (value: u64) -> u64
Foreign_Vararg_Proc :: proc "c" (fixed: u64, #c_vararg args: ..any) -> u64

foreign {
	foreign_procedure_value_target :: proc "c" (value: u64) -> u64 ---
	foreign_procedure_value_vararg_target :: proc "c" (fixed: u64, #c_vararg args: ..any) -> u64 ---
}

@(export)
foreign_procedure_value_fixed :: proc "c" (value: u64) -> u64 {
	target: Foreign_Proc = foreign_procedure_value_target
	return target(value)
}

@(export)
foreign_procedure_value_vararg :: proc "c" (value: u64) -> u64 {
	target: Foreign_Vararg_Proc = foreign_procedure_value_vararg_target
	return target(value, u64(1))
}

main :: proc() {}
