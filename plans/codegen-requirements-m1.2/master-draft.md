# M1.2 Backend Requirement Closure

- Status: complete
- Scope class: medium
- Last updated: 2026-08-02

## Problem

LLVM backend still reads global build context for code-generation policy. Backend contract must receive policy through immutable plan requirements.

## Scope

In scope: inventory every `build_context` read in `src/llvm_backend*.cpp`; classify policy, semantic body access, or driver/linker policy; move code-generation policy into `CodeGenRequirements`; replace LLVM policy reads with plan/session requirements.

Out of scope: CLI parsing, output paths, linker command policy, second backend, AST-free body plan.

## Dependencies

- M1.1 sealed program plan, session, backend dispatch, and plan dump.
- Existing LLVM backend object emission.

## Success Criteria

- LLVM code-generation policy reads come only from `CodeGenRequirements`.
- Driver/linker/output reads remain outside this cut.
- Release compiler builds.
- Object probes pass for none, minimal, debug, sanitizer, bounds, and cross-target modes.

## Milestones

### M1.2A - Requirement inventory

- Status: complete
- Files: all `src/llvm_backend*.cpp`, `src/codegen_plan.*`.
- Acceptance: every read classified with exact source location and target field decision.
- Verification: 204 reads inventoried: policy reads require requirements closure; output paths,
  temp retention, linker flags, and sanitizer runtime paths stay driver-owned; no semantic-body reads.

### M1.2B - Requirement closure

- Status: complete
- Depends on: M1.2A.
- Outcome: policy fields live in `CodeGenRequirements`; LLVM consumes plan/session values.

Completed closure: RTTI/assertion policy, dynamic literal/map policy, RVO and ABI switches,
Valgrind, source location and division policy, LLVM pass/verifier switches, Bedrock, diagnostics,
global-initializer policy, target layout, and module ownership all flow through requirements.
Target feature discovery moved to the driver before plan construction. LLVM-only source retains
`build_context` reads solely for output paths, temporary-file retention, and linker-driver flags.

Corrective: declaration-only imports of unspecialized polymorphic procedures no longer demand a
body definition from the work queue. Body requests retain the definition invariant.

### M1.2C - Backend matrix

- Status: complete
- Depends on: M1.2B.
- Outcome: focused object matrix proves policy closure keeps LLVM output viable.

Verified: release compiler; object-only `-o:none`, `-o:minimal`, `-o:minimal -no-bounds-check`,
`-debug`, `-sanitize:address`, and `-target:windows_amd64`; repeated plan dump SHA-256
`734b930cae8ce02d0af2bf7247eb68e68c6e3cbe04986a75366f50d2ab571846`.

### M1.2D - Dump boolean naming repair

- Status: complete
- Outcome: requirement-dump labels match their stored boolean semantics.
- Acceptance: `no_bounds_check`, `no_rtti`, and `no_type_assert` print under explicitly
  negative field names, or their values are inverted under positive names; the format remains
  deterministic and documented.

## Planning Horizon

- Current: M1.2 sealed.
- Next checkpoint: select the procedure semantic-source contract milestone.
- Deferred: CLI/output/linker policy remains outside this cut.

## Decisions

- `build_context` reads remain allowed for linker/driver/output policy in this cut.
- Semantic checker/AST access remains session-owned; no body-plan expansion.
- New immutable fields cover RTTI/type assertions, dynamic literals/maps, RVO, division behavior,
  source locations, internal ABI/pass/verifier switches, diagnostics, Bedrock, global-init, and Valgrind.

## Blockers and Follow-ups

- M1.2 sealed at `zonlvqsx`. Default requirements print `no-bounds=0`, `no-rtti=0`, and
  `no-type-assert=0`; disabled bounds/type assertions print `1`; Linux Bedrock prints
  `no-rtti=1`. Repeated default dumps matched SHA-256
  `12edcd7689a0c223d14bacf1e39649b057783fe69fef2346aa977cd88d4915dc`.
