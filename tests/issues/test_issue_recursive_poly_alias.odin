package test_issues

import "core:testing"

// DUMBAI: Checking the constructor argument reaches its own alias through a pointer cycle.
Worker_Deque :: Deque(Job, 4)
Deque :: struct($T: typeid, $N: u64) where N > 0 {
	buf: [N]T,
}
Job :: struct {
	group: ^Job_Group,
}
Job_Group :: struct {
	pool: ^Lazy_Pool,
}
Lazy_Pool :: struct {
	deques: []Worker_Deque,
}

Worker_Union :: Job_Union(Union_Job)
Job_Union :: union($T: typeid) {
	T,
}
Union_Job :: struct {
	pool: ^Union_Pool,
}
Union_Pool :: struct {
	jobs: []Worker_Union,
}

peek :: proc(deque: ^$D/Deque($T, $N)) -> T {
	return deque.buf[N-1]
}

@(test)
recursive_poly_alias :: proc(t: ^testing.T) {
	pool: Lazy_Pool
	group := Job_Group{pool = &pool}
	deque: Worker_Deque
	deque.buf[3] = Job{group = &group}
	testing.expect_value(t, peek(&deque).group.pool, &pool)

	union_pool: Union_Pool
	job := Worker_Union(Union_Job{pool = &union_pool})
	value, ok := job.(Union_Job)
	testing.expect(t, ok)
	testing.expect_value(t, value.pool, &union_pool)
}
