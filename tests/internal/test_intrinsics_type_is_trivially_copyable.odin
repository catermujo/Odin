package test_internal

import "base:intrinsics"
import "core:testing"

POD_Struct :: struct {
	x, y: int,
	z:    f32,
}

POD_Nested :: struct {
	a: POD_Struct,
	b: [4]u8,
	c: bit_set[0..<16],
}

Not_POD_Struct :: struct {
	x: int,
	s: string,
}

Struct_With_Pointer :: struct {
	x: int,
	p: ^int,
}

Struct_With_All_Pointers :: struct {
	a, b: ^int,
}

Array_Of_Pointers :: [3]^int
Array_Of_Mixed    :: [3]int
Union_With_Ptr    :: union { int, ^int }

Raw_Union_With_POD :: struct #raw_union {
	x: int,
	y: f32,
	z: [3]u8,
}

Union_Of_POD :: union {
	int,
	f32,
	[2]u16,
}

Union_With_String :: union {
	int,
	string,
}

Simple_Bit_Field :: bit_field u16 {
	a: u8 | 4,
	b: u8 | 4,
}

@(test)
test_intrinsics_type_is_trivially_copyable_positives :: proc(t: ^testing.T) {
	// Basic scalars and numerics.
	testing.expect(t,  intrinsics.type_is_trivially_copyable(int))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(u64))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(f32))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(bool))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(rune))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(byte))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(typeid))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(b16))

	// Pointer-shaped primitives are POD (pointers are ok to copy).
	testing.expect(t,  intrinsics.type_is_trivially_copyable(rawptr))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(uintptr))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(^int))
	testing.expect(t,  intrinsics.type_is_trivially_copyable([^]int))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(proc() -> int))     // bare proc
	testing.expect(t,  intrinsics.type_is_trivially_copyable(proc "contextless" () -> int))

	// Enums / bit sets / bit fields.
	testing.expect(t,  intrinsics.type_is_trivially_copyable(enum { A, B, C }))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(bit_set[0..<8]))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(bit_set[0..<64; u64]))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(Simple_Bit_Field))

	// Aggregates of POD, no pointer field anywhere.
	testing.expect(t,  intrinsics.type_is_trivially_copyable([4]int))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(POD_Struct))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(POD_Nested))

	// SIMD vectors and matrices.
	testing.expect(t,  intrinsics.type_is_trivially_copyable(#simd[4]f32))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(matrix[3, 3]f32))
	testing.expect(t,  intrinsics.type_is_trivially_copyable([dynamic; 8]int)) // fixed-capacity, no heap header

	// SOA struct (no indirection in lane data).
	testing.expect(t,  intrinsics.type_is_trivially_copyable(#soa[4]POD_Struct))

	// Unions and raw unions of POD variants.
	testing.expect(t,  intrinsics.type_is_trivially_copyable(Union_Of_POD))
	testing.expect(t,  intrinsics.type_is_trivially_copyable(Raw_Union_With_POD))

	// Distinct types follow the underlying type.
	My_Int :: distinct int
	testing.expect(t,  intrinsics.type_is_trivially_copyable(My_Int))
}

@(test)
test_intrinsics_type_is_trivially_copyable_negatives :: proc(t: ^testing.T) {
	// Managed / heap-tracked types.
	testing.expect(t, !intrinsics.type_is_trivially_copyable(string))
	testing.expect(t, !intrinsics.type_is_trivially_copyable(cstring))
	testing.expect(t, !intrinsics.type_is_trivially_copyable([]int))
	testing.expect(t, !intrinsics.type_is_trivially_copyable([dynamic]int))
	testing.expect(t, !intrinsics.type_is_trivially_copyable(map[string]int))
	testing.expect(t, !intrinsics.type_is_trivially_copyable(any))

	// Structs: any pointer-typed field disqualifies the whole struct.
	testing.expect(t, !intrinsics.type_is_trivially_copyable(Not_POD_Struct))     // has string
	testing.expect(t, !intrinsics.type_is_trivially_copyable(Struct_With_Pointer)) // has ^int
	testing.expect(t, !intrinsics.type_is_trivially_copyable(Struct_With_All_Pointers))
	testing.expect(t, !intrinsics.type_is_trivially_copyable([]string))
	testing.expect(t, !intrinsics.type_is_trivially_copyable(Union_With_String))
	testing.expect(t, !intrinsics.type_is_trivially_copyable(Union_With_Ptr))      // union has ^int variant

	// Arrays: pointer-typed element disqualifies the whole array.
	testing.expect(t, !intrinsics.type_is_trivially_copyable(Array_Of_Pointers))
	testing.expect(t, !intrinsics.type_is_trivially_copyable([4]string))
	testing.expect(t, !intrinsics.type_is_trivially_copyable([dynamic; 4]string))
	testing.expect(t, !intrinsics.type_is_trivially_copyable([dynamic; 4]^int))

	// Sanity: an array of pure-int is still POD (regression check).
	testing.expect(t,  intrinsics.type_is_trivially_copyable(Array_Of_Mixed))
}

// Verify the predicate is actually used at compile time: the conditional
// is folded to a constant and the unused branch must be elided.
@(test)
test_intrinsics_type_is_trivially_copyable_consteval :: proc(t: ^testing.T) {
	// Compile-time constants.
	I_AM_POD     :: intrinsics.type_is_trivially_copyable(int)
	I_AM_NOT_POD :: intrinsics.type_is_trivially_copyable(string)
	PTR_IS_POD   :: intrinsics.type_is_trivially_copyable(^int)
	STRUCT_OF_PTR_NOT_POD :: intrinsics.type_is_trivially_copyable(struct { p: ^int })
	testing.expect_value(t, I_AM_POD,                true)
	testing.expect_value(t, I_AM_NOT_POD,            false)
	testing.expect_value(t, PTR_IS_POD,              true)
	testing.expect_value(t, STRUCT_OF_PTR_NOT_POD,   false)
}
