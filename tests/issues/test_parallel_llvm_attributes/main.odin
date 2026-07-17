package test_parallel_llvm_attributes

main :: proc() {
	boxed: any = u8(1)
	_ = boxed.(u8)
}
