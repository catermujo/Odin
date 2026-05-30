# Force-inline warning opt-out

- Status: complete
- Scope class: tiny
- Last updated: 2026-08-09

## Problem

Some intentional `#force_inline` procedures trigger excessive-inlining warnings even when the warning is not useful. A source-level opt-out is needed without changing LLVM inlining behavior.

## Scope

### In scope

- Add `@(no_warn_excessive_inlining)` for procedure declarations.
- Propagate it through polymorphic procedure specializations.
- Skip only the excessive-inlining diagnostic.

### Out of scope

- Changing `#force_inline` behavior.
- Adding name-based suppression or a generic warning framework.

## Dependencies

- Existing declaration attribute handling.
- Existing `lbProcedure` metadata used by the LLVM warning analyzer.

## Success criteria

- Annotated procedures do not produce excessive-inlining warnings.
- Unannotated procedures still produce warnings under existing thresholds.
- The compiler builds successfully.

## Milestones

### M1 - Procedure suppression metadata

- Status: complete
- Files: `src/checker.hpp`, `src/checker.cpp`, `src/check_decl.cpp`, `src/check_expr.cpp`, `src/entity.cpp`, `src/llvm_backend_proc.cpp`, `src/llvm_backend.hpp`, `src/llvm_backend.cpp`
- Acceptance criteria: attribute parses, propagates to generated procedures, and analyzer skips only marked procedures.
- Verification: `./build_odin.sh release-native`; `./odin check tests/issues/test_issue_no_warn_excessive_inlining.odin -file`; `./tool build conurbation -verbose` — all passed.

## Planning horizon

- Current milestone: M1 - Procedure suppression metadata.
- Next checkpoint: build and run focused fixture.
- Deferred detail: none.

## Decisions

- Use declaration attribute metadata because suppression is diagnostic policy, not optimization semantics.
- Keep global `-no-warn-excessive-inlining` unchanged.

## Blockers and follow-ups

- None.
