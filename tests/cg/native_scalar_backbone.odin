package native_scalar_backbone

foreign {
	native_backbone_external_add :: proc "c" (lhs, rhs: i32) -> i32 ---
	native_backbone_external_void :: proc "c" () ---
}

native_backbone_internal_add :: proc "c" (lhs, rhs: i32) -> i32 {
	return lhs + rhs
}

native_backbone_internal_eight :: proc "c" (a, b, c, d, e, f, g, h: i64) -> i64 {
	return a + h
}

@(export)
native_backbone_multiple_calls :: proc "c" (value: i32) -> i32 {
	local: i32
	local = value + 1
	native_backbone_internal_add(local, 2)
	local = local + 3
	return native_backbone_external_add(local, 4)
}

@(export)
native_backbone_multiple_void_calls :: proc "c" (value: i32) {
	local: i32
	local = value + 1
	native_backbone_external_void()
	local = local + 2
	native_backbone_external_void()
}

@(export)
native_backbone_liveness_pair :: proc "c" (lhs, rhs: i32) -> i32 {
	native_backbone_external_void()
	return native_backbone_internal_add(lhs, rhs)
}

@(export)
native_backbone_liveness_eight :: proc "c" (a, b, c, d, e, f, g, h: i64) -> i64 {
	native_backbone_external_void()
	return native_backbone_internal_eight(a, b, c, d, e, f, g, h)
}

@(export)
native_backbone_if_merge :: proc "c" (condition, value: i32) -> i32 {
	local: i32
	if condition != 0 {
		local = value + 1
	} else {
		local = value - 1
	}
	return local
}

@(export)
native_backbone_if_returns :: proc "c" (condition, value: i32) -> i32 {
	local: i32
	if condition != 0 {
		local = value + 1
		return local + 2
	} else {
		local = value - 1
		return local - 2
	}
}

main :: proc() {}
