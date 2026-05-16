# Odin Catermujo fork

This is an light fork of the compiler. Our goal is to get as much of the mods here merged in one way or the other and
try out ideas that work well for math-heavy code while getting as much safety as possible in debug builds.

## Features

- Reintroduction of #no_copy attribute which vets copying structs at compile-time. Very useful for mutexes and arenas
  which cannot be safely copied. (Won't be upstreamed)
- `-emit-downcast-assert` wraps every possible wrong integer cast so you get proper assertions when numbers don't fit
  nicely (can be bypassed using `#no_downcast_assert`, exports `ODIN_DOWNCAST_ASSERT` constant too).
- Better array programming: first we avoid copying arrays to the stack when doing broadcasting operations which allows
  you to use any array size you need. Bonus features: can do array-programming on slices + auto SIMD-unrolls in the compiler.
- Operator overloading (with `@(operator)` annotation). Supported ops: `+`, `-`, `*`, `/` and container getter `[]`,
  setter `[]=` and iteration `in`.
- `#optional_{ok,allocator_error}` emits assertions when the optional parameter is ignored.
- We completely skip checking `@(disabled)` procs when they are disabled. Allows more confident use of the feature.
- Clojure support `x: int; y := lambda[&x]() { x += 1 }`
- Better support for wasm and emscripten.
- More freedom to slice and do array programming with dense zero-start enum arrays
- Compile-time `#assert`/`#panic` traces.
- Minor improvements to comp-time (`where` expressions actually change what can be returned).
- Generic `#+when` for gating an entire file.
- Policy-driven scope exits (replaces `@(deferred_*)`): `.explicit` needs to create a scope with either `if`, `for` or
  `with` (new). `.implicit` keeps current behavior.
- Many fixes, see tree from `upstream/master..origin/master`.
