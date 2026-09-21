package CoreFoundation

Dictionary :: distinct TypeRef
Boolean    :: distinct TypeRef

foreign import CoreFoundation "system:CoreFoundation.framework"

@(link_prefix = "CF", default_calling_convention = "c")
foreign CoreFoundation {
	DictionaryGetValue :: proc(dictionary: Dictionary, key: rawptr) -> rawptr ---
	BooleanGetValue    :: proc(value: Boolean) -> b8 ---
}
