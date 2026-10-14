#include "llvm-c/Core.h"

gb_internal void lb_llvm_set_hidden_visibility(LLVMValueRef value) {
	// DUMBAI: LLVM requires default visibility for local symbols; weak cross-module definitions still need hiding.
	LLVMLinkage linkage = LLVMGetLinkage(value);
	LLVMSetVisibility(value, linkage == LLVMInternalLinkage || linkage == LLVMPrivateLinkage
		? LLVMDefaultVisibility : LLVMHiddenVisibility);
}
