package main
call_c :: proc() -> bool { a: []i64 = {1,2,3}; b: []u64 = {1,2,3}; _, a_ok := find_any_index(a, accept_i64); _, b_ok := find_any_index(b, accept_u64); return a_ok && b_ok }
