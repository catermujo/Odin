package test_issue_no_warn_excessive_inlining

@(no_warn_excessive_inlining)
suppressed :: #force_inline proc() {}

@(no_warn_excessive_inlining)
suppressed_poly :: #force_inline proc(value: $T) -> T { return value }

main :: proc() { _ = suppressed_poly(42) }
