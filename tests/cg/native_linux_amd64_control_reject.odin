package native_linux_amd64_control_reject

foreign {
	native_linux_amd64_control_foreign :: proc "c" (value: u64) -> u64 ---
}

Native_Linux_AMD64_Control_Callback :: proc "c" (value: u64) -> u64

@(export)
native_linux_amd64_control_reject :: proc "c" (value: u64) -> u64 {
	if value != 0 {
		callback: Native_Linux_AMD64_Control_Callback = native_linux_amd64_control_foreign
		return callback(value)
	}
	return value
}
