# Force-inline warning threshold

- Status: complete
- Scope class: tiny
- Last updated: 2026-08-09

## Problem

Repeatedly inlining a small leaf helper is often intentional and beneficial. The excessive-inlining warning should focus on helpers whose optimized bodies carry structural cost.

## Scope

### In scope

- Count final-IR instructions, branch instructions, pointer or memory operations, and residual call instructions.
- Skip procedures with only one aggregated pre-module call site.
- Warn only when a body has at least 256 instructions and has branching, pointer/memory access, or more than two residual calls.
- Preserve existing warning aggregation and estimated-expansion ordering.

### Out of scope

- LLVM source changes.
- Changing `#force_inline` behavior or call-site counts.

## Dependencies

- Existing final-module warning analyzer in `src/llvm_backend.cpp`.

## Success criteria

- Small leaf helpers called many times no longer warn solely due to expansion estimate.
- One-off force-inline procedures do not warn.
- Structurally expensive bodies at or above 256 instructions still warn.
- Compiler and conurbation builds pass.

## Milestones

### M1 - Structural warning threshold

- Status: complete
- Files: `src/llvm_backend.cpp`
- Acceptance criteria: analyzer applies the new predicate and preserves warning ordering.
- Verification: `./build_odin.sh release-native`; `./odin check tests/issues/test_issue_no_warn_excessive_inlining.odin -file`; `./tool build conurbation -verbose` — all passed. `encode_to_marshal_err` no longer warned despite 68 pre-module call sites.

## Planning horizon

- Current milestone: M1 - Structural warning threshold.
- Next checkpoint: compare warning list and counts against previous run.
- Deferred detail: none.

## Decisions

- Use final optimized IR for structural signals because it reflects what LLVM kept after inlining and simplification.
- Count residual call-like instructions as non-inlined calls.
- Treat one aggregated pre-module call site as insufficient evidence for a warning.
- Aggregate structural signals conservatively across generated instances at one source location.

## Blockers and follow-ups

- None.
