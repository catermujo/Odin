// DUMBAI: Contract checks must run even when llvm-config supplies release flags with NDEBUG.
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "llvm-c/Analysis.h"
#include "llvm-c/Core.h"
#include "llvm-c/IRReader.h"

#define gb_internal static
#include "llvm_visibility.hpp"

static void check_roundtrip(LLVMModuleRef module) {
	char *text = LLVMPrintModuleToString(module);
	LLVMContextRef context = LLVMContextCreate();
	LLVMMemoryBufferRef buffer = LLVMCreateMemoryBufferWithMemoryRangeCopy(text, strlen(text), "visibility");
	LLVMModuleRef parsed = nullptr;
	char *error = nullptr;
	assert(LLVMParseIRInContext(context, buffer, &parsed, &error) == 0);
	assert(LLVMVerifyModule(parsed, LLVMReturnStatusAction, &error) == 0);
	LLVMDisposeMessage(error);
	LLVMDisposeModule(parsed);
	LLVMContextDispose(context);
	LLVMDisposeMessage(text);
}

int main(int argc, char **argv) {
	LLVMContextRef context = LLVMContextCreate();
	LLVMModuleRef module = LLVMModuleCreateWithNameInContext("visibility", context);
	LLVMLinkage linkages[] = {LLVMInternalLinkage, LLVMPrivateLinkage, LLVMWeakAnyLinkage, LLVMExternalLinkage};
	for (int i = 0; i < 4; i++) {
		char global_name[32], function_name[32];
		snprintf(global_name, sizeof(global_name), "global_%d", i);
		snprintf(function_name, sizeof(function_name), "function_%d", i);
		LLVMValueRef global = LLVMAddGlobal(module, LLVMInt32TypeInContext(context), global_name);
		LLVMSetInitializer(global, LLVMConstInt(LLVMInt32TypeInContext(context), 1, false));
		LLVMValueRef function = LLVMAddFunction(module, function_name,
			LLVMFunctionType(LLVMVoidTypeInContext(context), nullptr, 0, false));
		LLVMBuilderRef builder = LLVMCreateBuilderInContext(context);
		LLVMPositionBuilderAtEnd(builder, LLVMAppendBasicBlockInContext(context, function, "entry"));
		LLVMBuildRetVoid(builder);
		LLVMDisposeBuilder(builder);
		LLVMValueRef values[] = {global, function};
		for (LLVMValueRef value : values) {
			LLVMSetLinkage(value, linkages[i]);
			lb_llvm_set_hidden_visibility(value);
			assert(LLVMGetVisibility(value) == (i < 2 ? LLVMDefaultVisibility : LLVMHiddenVisibility));
		}
	}
	check_roundtrip(module);
	LLVMDisposeModule(module);
	LLVMContextDispose(context);
	for (int input = 1; input < argc; input++) {
		context = LLVMContextCreate();
		LLVMMemoryBufferRef buffer = nullptr;
		char *error = nullptr;
		if (LLVMCreateMemoryBufferWithContentsOfFile(argv[input], &buffer, &error) ||
			LLVMParseIRInContext(context, buffer, &module, &error)) {
			fprintf(stderr, "%s\n", error);
			LLVMDisposeMessage(error);
			LLVMContextDispose(context);
			return 1;
		}
		assert(LLVMVerifyModule(module, LLVMReturnStatusAction, &error) == 0);
		LLVMDisposeMessage(error);
		LLVMDisposeModule(module);
		LLVMContextDispose(context);
	}
	puts("LLVM IR visibility passed");
	return 0;
}
