package native_integer_ops

@(export)
native_i32_add :: proc(lhs, rhs: i32) -> i32 { return lhs + rhs }
@(export)
native_i32_sub :: proc(lhs, rhs: i32) -> i32 { return lhs - rhs }
@(export)
native_i32_mul :: proc(lhs, rhs: i32) -> i32 { return lhs * rhs }
@(export)
native_i32_and :: proc(lhs, rhs: i32) -> i32 { return lhs & rhs }
@(export)
native_i32_or :: proc(lhs, rhs: i32) -> i32 { return lhs | rhs }
@(export)
native_i32_xor :: proc(lhs, rhs: i32) -> i32 { return lhs ~ rhs }
@(export)
native_i32_add_const :: proc(lhs: i32) -> i32 { return lhs + 7 }
@(export)
native_i32_nested :: proc(a, b, c: i32) -> i32 { return a + b*c }
@(export)
native_i32_shared :: proc(a, b: i32) -> i32 { return (a + b) * (a + b) }

@(export)
native_u32_add :: proc(lhs, rhs: u32) -> u32 { return lhs + rhs }
@(export)
native_u32_sub :: proc(lhs, rhs: u32) -> u32 { return lhs - rhs }
@(export)
native_u32_mul :: proc(lhs, rhs: u32) -> u32 { return lhs * rhs }
@(export)
native_u32_and :: proc(lhs, rhs: u32) -> u32 { return lhs & rhs }
@(export)
native_u32_or :: proc(lhs, rhs: u32) -> u32 { return lhs | rhs }
@(export)
native_u32_xor :: proc(lhs, rhs: u32) -> u32 { return lhs ~ rhs }

@(export)
native_i64_add :: proc(lhs, rhs: i64) -> i64 { return lhs + rhs }
@(export)
native_i64_sub :: proc(lhs, rhs: i64) -> i64 { return lhs - rhs }
@(export)
native_i64_mul :: proc(lhs, rhs: i64) -> i64 { return lhs * rhs }
@(export)
native_i64_and :: proc(lhs, rhs: i64) -> i64 { return lhs & rhs }
@(export)
native_i64_or :: proc(lhs, rhs: i64) -> i64 { return lhs | rhs }
@(export)
native_i64_xor :: proc(lhs, rhs: i64) -> i64 { return lhs ~ rhs }
@(export)
native_i64_xor_const :: proc(lhs: i64) -> i64 { return lhs ~ 0x1122334455667788 }
@(export)
native_i64_add_8 :: proc(a, b, c, d, e, f, g, h: i64) -> i64 { return g + h }
@(export)
native_i64_nested :: proc(a, b, c, d: i64) -> i64 { return (a + b) * (c - d) }
@(export)
native_i64_deep_constants :: proc(a: i64) -> i64 { return ((((((a + 1) + 2) + 3) + 4) + 5) + 6) + 7 }

@(export)
native_u64_add :: proc(lhs, rhs: u64) -> u64 { return lhs + rhs }
@(export)
native_u64_sub :: proc(lhs, rhs: u64) -> u64 { return lhs - rhs }
@(export)
native_u64_mul :: proc(lhs, rhs: u64) -> u64 { return lhs * rhs }
@(export)
native_u64_and :: proc(lhs, rhs: u64) -> u64 { return lhs & rhs }
@(export)
native_u64_or :: proc(lhs, rhs: u64) -> u64 { return lhs | rhs }
@(export)
native_u64_xor :: proc(lhs, rhs: u64) -> u64 { return lhs ~ rhs }

main :: proc() {}
