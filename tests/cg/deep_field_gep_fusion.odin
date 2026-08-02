package main

Leaf :: struct {
	padding: u16,
	value:   i64,
}

Middle :: struct {
	leaf: Leaf,
}

Outer :: struct {
	middle: Middle,
}

Packed_Leaf :: struct #packed {
	padding: u8,
	value:   i32,
}

Packed_Middle :: struct {
	using leaf: Packed_Leaf,
}

Packed_Outer :: struct {
	using middle: Packed_Middle,
}

Using_Leaf :: struct {
	padding: u16,
	value:   i64,
}

Using_Middle :: struct {
	using leaf: Using_Leaf,
}

Using_Outer :: struct {
	using middle: Using_Middle,
}

Pointer_Middle :: struct {
	using leaf: ^Using_Leaf,
}

Pointer_Outer :: struct {
	using middle: Pointer_Middle,
}

main :: proc() {
	value: Outer
	value.middle.leaf.value = 42
	assert(value.middle.leaf.value == 42)

	packed: Packed_Outer
	packed.value = 24
	assert(packed.value == 24)

	using_value: Using_Outer
	using_value.value = 84
	assert(using_value.value == 84)

	using_ptr := &using_value
	using_ptr.value = 168
	assert(using_ptr.value == 168)

	pointer_leaf: Using_Leaf
	pointer_value: Pointer_Outer
	pointer_value.leaf = &pointer_leaf
	pointer_value.value = 21
	assert(pointer_value.value == 21)
}
