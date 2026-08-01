package native_linux_amd64_context_reject

@(export)
native_linux_amd64_context_reject_member :: proc(value: u64) -> u64 {
	return value + u64(context.user_index)
}
