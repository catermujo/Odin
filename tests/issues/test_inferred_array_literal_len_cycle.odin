package test_issues

Feature_Descriptor :: struct {
	callback: Feature_Callback,
}

FEATURE_DESCRIPTORS := [?]Feature_Descriptor{{feature_callback}}

Feature_Instance :: struct {
	descriptor: ^Feature_Descriptor,
}

Feature_Instance_Set :: struct {
	instances: [len(FEATURE_DESCRIPTORS)]Feature_Instance,
}

Feature_Context :: struct {
	instances: ^Feature_Instance_Set,
}

Feature_Callback :: #type proc(ctx: ^Feature_Context) -> bool

feature_callback :: proc(ctx: ^Feature_Context) -> bool {
	return ctx != nil
}
