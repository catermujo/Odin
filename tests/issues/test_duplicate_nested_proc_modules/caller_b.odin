package main
call_b :: proc() -> bool { items: []i32 = {1, 2, 3}; _, ok := find_any_index(items, accept_i32); return ok }
