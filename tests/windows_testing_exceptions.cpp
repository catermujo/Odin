#include <windows.h>
#include <intrin.h>
#include <stdexcept>

static DWORD const PREDECESSOR_CODE = 0xe0425142;
static LPTOP_LEVEL_EXCEPTION_FILTER old_filter;
static bool predecessor_installed;

// DUMBAI: This predecessor recovers only the fixture's continuable helper-thread exception.
static LONG WINAPI predecessor(EXCEPTION_POINTERS *info) {
	if (info->ExceptionRecord->ExceptionCode == static_cast<DWORD>(PREDECESSOR_CODE)) {
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return old_filter ? old_filter(info) : EXCEPTION_CONTINUE_SEARCH;
}

extern "C" __declspec(dllexport) void probe_install_predecessor() {
	old_filter = SetUnhandledExceptionFilter(predecessor);
	predecessor_installed = true;
}

// DUMBAI: No exception crosses this export's C ABI; the DLL must finish its own catch normally.
extern "C" __declspec(dllexport) int probe_caught_cpp() {
	try {
		throw std::runtime_error("internally caught C++ exception");
	} catch (std::runtime_error const &) {
		return 42;
	}
}

extern "C" __declspec(dllexport) int probe_caught_seh() {
	__try {
		RaiseException(0xe0421234, 0, 0, nullptr);
	} __except (GetExceptionCode() == 0xe0421234 ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
		return 43;
	}
	return 0;
}

extern "C" __declspec(dllexport) int probe_caught_access_violation() {
	__try {
		return *reinterpret_cast<volatile int *>(1);
	} __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
		return 44;
	}
}

static DWORD WINAPI child_predecessor(void *) {
	RaiseException(static_cast<DWORD>(PREDECESSOR_CODE), 0, 0, nullptr);
	return 45;
}

extern "C" __declspec(dllexport) int probe_child_predecessor() {
	HANDLE thread = CreateThread(nullptr, 0, child_predecessor, nullptr, 0, nullptr);
	if (!thread) return 0;
	DWORD result = 0;
	if (WaitForSingleObject(thread, 10000) == WAIT_OBJECT_0) GetExitCodeThread(thread, &result);
	CloseHandle(thread);
	return static_cast<int>(result);
}

extern "C" __declspec(dllexport) void probe_unhandled_access_violation() {
	*reinterpret_cast<volatile int *>(1) = 7;
}

extern "C" __declspec(dllexport) void probe_unhandled_illegal_instruction() {
	__ud2();
}

extern "C" __declspec(dllexport) void probe_unhandled_arithmetic() {
	volatile int numerator = 1;
	volatile int denominator = 0;
	volatile int result = numerator / denominator;
	(void)result;
}

extern "C" __declspec(dllexport) void probe_unhandled_high_code() {
	RaiseException(0xe0429876, 0, 0, nullptr);
}

extern "C" __declspec(dllexport) void probe_unhandled_low_code() {
	RaiseException(0x20429876, 0, 0, nullptr);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
	if (reason == DLL_PROCESS_DETACH && predecessor_installed) {
		// DUMBAI: Odin test entry exits directly, so verify restoration at DLL detach after worker shutdown.
		auto current = SetUnhandledExceptionFilter(predecessor);
		SetUnhandledExceptionFilter(current);
		char const good[] = "predecessor-restored=yes\n";
		char const bad[] = "predecessor-restored=no\n";
		DWORD written;
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), current == predecessor ? good : bad,
		          current == predecessor ? sizeof(good) - 1 : sizeof(bad) - 1, &written, nullptr);
	}
	return TRUE;
}
