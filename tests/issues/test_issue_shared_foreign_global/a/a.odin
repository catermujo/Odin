package test_issue_shared_foreign_global_a

when ODIN_OS == .Darwin {
	foreign import lib "system:System.framework"

	foreign lib {
		vm_page_size: uintptr
	}

	shared_global :: proc() -> uintptr {
		return vm_page_size
	}
} else {
	foreign import lib "system:c"

	foreign lib {
		environ: rawptr
	}

	shared_global :: proc() -> uintptr {
		return uintptr(environ)
	}
}
