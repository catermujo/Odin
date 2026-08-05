package test_issues

Inner_Left :: enum {
	a,
	b,
}

Inner_Right :: enum {
	c,
	d,
}

Inner :: union {
	Inner_Left,
	Inner_Right,
}

Outer :: union {
	Inner,
	int,
}

Scalar_Union :: union {
	int,
	f32,
}

Atom :: struct {
	token: Outer,
}

Variant_Record :: struct {
	value:  Scalar_Union,
	marker: int,
}

NAMED_INNER :: Inner(Inner_Left.a)

DIRECT_ATOMS :: [?]Atom{{token = Inner(Inner_Left.a)}}
NAMED_ATOMS :: [?]Atom{{token = NAMED_INNER}}

INDEXED_OUTERS :: [1]Outer {
	0 = Inner(Inner_Left.a),
}

RANGED_OUTERS :: [1]Outer {
	0..=0 = Inner(Inner_Left.a),
}

Outer_Index :: enum {first}

ENUMERATED_OUTERS :: [Outer_Index]Outer {
	.first = Inner(Inner_Left.a),
}

RANGED_ENUMERATED_OUTERS :: [Outer_Index]Outer {
	.first..=.first = Inner(Inner_Left.a),
}

FIXED_CAPACITY_OUTERS :: [dynamic; 1]Outer{
	0 = Inner(Inner_Left.a),
}

RANGED_FIXED_CAPACITY_OUTERS :: [dynamic; 1]Outer{
	0..=0 = Inner(Inner_Left.a),
}

POSITIONAL_FIXED_CAPACITY_OUTERS :: [dynamic; 1]Outer{
	Inner(Inner_Left.a),
}

COMPARISON_A : Outer : Inner(Inner_Left.a)
COMPARISON_B : Outer : Inner(Inner_Left.a)
COMPARISON_C : Outer : Inner(Inner_Left.b)
COMPARISON_D : Scalar_Union : int(1)
COMPARISON_E : Scalar_Union : f32(1)
COMPARISON_TAGS_DIFFER :: COMPARISON_D != COMPARISON_E

COMPOUND_COMPARISON_A : [2]Outer : {Inner(Inner_Left.a), Inner(Inner_Left.b)}
COMPOUND_COMPARISON_B : [2]Outer : {Inner(Inner_Left.a), Inner(Inner_Left.b)}
COMPOUND_COMPARISON_SAME :: COMPOUND_COMPARISON_A == COMPOUND_COMPARISON_B
COMPOUND_TAGS_A : [2]Scalar_Union : {int(1), int(2)}
COMPOUND_TAGS_B : [2]Scalar_Union : {f32(1), f32(2)}
COMPOUND_TAGS_DIFFER :: COMPOUND_TAGS_A != COMPOUND_TAGS_B
RECORD_A : Variant_Record : {value = int(1), marker = 2}
RECORD_B : Variant_Record : {value = int(1), marker = 2}
RECORD_C : Variant_Record : {value = f32(1), marker = 2}
RECORD_SAME :: RECORD_A == RECORD_B
RECORD_TAGS_DIFFER :: RECORD_A != RECORD_C
BROADCAST_OUTERS : [2]Outer : Inner(Inner_Left.a)

main :: proc() {
	_ = DIRECT_ATOMS
	_ = NAMED_ATOMS
	_ = INDEXED_OUTERS
	_ = RANGED_OUTERS
	_ = ENUMERATED_OUTERS
	_ = RANGED_ENUMERATED_OUTERS
	_ = FIXED_CAPACITY_OUTERS
	_ = RANGED_FIXED_CAPACITY_OUTERS
	_ = POSITIONAL_FIXED_CAPACITY_OUTERS
	assert(COMPARISON_A == COMPARISON_B)
	assert(COMPARISON_A != COMPARISON_C)
	assert(COMPARISON_D != COMPARISON_E)
	assert(COMPARISON_TAGS_DIFFER)
	assert(COMPOUND_COMPARISON_A == COMPOUND_COMPARISON_B)
	assert(COMPOUND_COMPARISON_SAME)
	assert(COMPOUND_TAGS_DIFFER)
	assert(RECORD_SAME)
	assert(RECORD_TAGS_DIFFER)
	_ = BROADCAST_OUTERS
	@(static) static_broadcast : [2]Outer = Inner(Inner_Left.a)
	assert(static_broadcast[0] == Outer(Inner(Inner_Left.a)))
	assert(static_broadcast[1] == Outer(Inner(Inner_Left.a)))
	runtime_int := Scalar_Union(int(1))
	runtime_float := Scalar_Union(f32(1))
	assert(runtime_int != runtime_float)
	assert(runtime_int == Scalar_Union(int(1)))
}
