package test_issue_shared_foreign_global_b

when ODIN_OS == .Darwin {
	foreign import lib "system:System.framework"

	foreign lib {
		@(link_name="vm_page_size") value: u64
	}
} else {
	foreign import lib "system:c"

	foreign lib {
		@(link_name="environ") value: [^]cstring
	}
}

shared_global :: proc() -> uintptr {
	return uintptr(value)
}
