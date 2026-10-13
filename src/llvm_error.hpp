#ifndef ODIN_LLVM_ERROR_HPP
#define ODIN_LLVM_ERROR_HPP

#include "llvm-c/Error.h"

gb_internal void lb_llvm_print_pass_error(LLVMErrorRef error) {
	// DUMBAI: LLVMGetErrorMessage consumes the error; only its returned message still needs disposal.
	char *message = LLVMGetErrorMessage(error);
	gb_printf_err("LLVM Error:\n%s\n", message);
	LLVMDisposeErrorMessage(message);
}

#endif
