package deferred_backend

when BACKEND != BACKEND_VULKAN {
	Native_Vulkan_Buffer :: struct {}
}
