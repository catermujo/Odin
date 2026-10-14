// DUMBAI: Contract checks must run even when llvm-config supplies release flags with NDEBUG.
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include "llvm-c/Core.h"
#include "llvm-c/Transforms/PassBuilder.h"

#define gb_internal static
#define gb_printf_err(...) fprintf(stderr, __VA_ARGS__)
#include "llvm_error.hpp"

int main() {
	LLVMContextRef context = LLVMContextCreate();
	LLVMModuleRef module = LLVMModuleCreateWithNameInContext("error-cleanup", context);
	LLVMPassBuilderOptionsRef options = LLVMCreatePassBuilderOptions();
	for (int i = 0; i < 3; i++) {
		LLVMErrorRef error = LLVMRunPasses(module, "odin-intentionally-invalid-pass", nullptr, options);
		assert(error != nullptr);
		lb_llvm_print_pass_error(error);
		LLVMErrorRef success = LLVMRunPasses(module, "verify", nullptr, options);
		assert(success == nullptr);
	}
	LLVMDisposePassBuilderOptions(options);
	LLVMDisposeModule(module);
	LLVMContextDispose(context);
	puts("LLVM pass error cleanup passed");
	return 0;
}
