package test_issue_deferred_build_tag_imported_constant

when ENABLE {
	#assert(VALUE == 1)
} else {
	VALUE :: 0
	#assert(VALUE == 0)
}
