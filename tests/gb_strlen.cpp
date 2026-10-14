#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#define GB_STATIC
#define GB_IMPLEMENTATION
// DUMBAI: The standalone header includes varargs helpers that GCC cannot always-inline under AddressSanitizer.
#define gb_inline inline
#include "gb/gb.h"

// DUMBAI: The monolithic header requires this provider; string length never allocates through it.
gb_internal gbAllocator heap_allocator(void) { std::abort(); }
gb_internal void print_all_errors(void) {}
gb_internal bool any_errors(void) { return false; }
gb_internal bool any_warnings(void) { return false; }

int main() {
	isize (*volatile length)(char const *) = gb_strlen;
	assert(length(nullptr) == 0);
	assert(length("") == 0);
	assert(length("intrinsics") == 10);
	for (isize count = 0; count < 64; count++) {
		char *text = static_cast<char *>(std::malloc(static_cast<size_t>(count + 1)));
		std::memset(text, 'x', static_cast<size_t>(count));
		text[count] = 0;
		assert(length(text) == count);
		std::free(text);
	}
	std::puts("String object length bounds passed");
}
