# Codegen Session M1.1

- Status: complete
- Scope class: large
- Last updated: 2026-08-02

## Problem

The immutable program plan must have a safe dump boundary and a backend-neutral live session for dynamic work.

## Scope

In scope: atomic plan dumps, `CodeGenSession`, session-owned dynamic procedure work, backend registry dispatch, and LLVM consumption of planned metadata.

Out of scope: AST-free body IR and any second backend.

## Dependencies

- Sealed `CodeGenProgramPlan` foundation.
- Existing LLVM backend and linker contract.

## Success Criteria

- Concurrent dumps do not corrupt or truncate the destination.
- Failed dump writes preserve an existing destination.
- Plan remains immutable while `CodeGenSession` owns checker/AST handles and dynamic work.
- LLVM is selected through a generic registry and consumes planned metadata.
- Focused object and dump gates pass.

## Milestones

### M1.1A - Dump commit protocol

- Status: complete
- Files: `src/codegen_plan.cpp`
- Acceptance: unique exclusive same-directory temporary file; complete write; flush/close; atomic replacement; `no-thread-local` label.
- Verification: repeated and concurrent dump commands, failed destination preservation.

### M1.1B - Session and dynamic procedure ownership

- Status: complete
- Outcome: session owns checker/AST lifetime, static ABI seed, and dynamic declaration/body queue.

### M1.1C - Backend dispatch and LLVM plan consumption

- Status: complete
- Outcome: registry selects LLVM and LLVM consumes plan metadata without semantic rereads.

### M1.1C1 - Foreign-link ordering repair

- Status: complete
- Files: `src/codegen_plan.*`, `src/linker.cpp`, focused plan-order probe.
- Acceptance: plan records a stable declaration identity plus legacy order keys; the linker sorts
  by those keys and deduplicates only matching declaration identities.
- Verification: release build, object-only none/minimal probes, deterministic dump, and a
  duplicate-payload/equal-priority ordering probe.

## Decisions

- The plan stays immutable. Dynamic declarations are session records, not mutations of the plan.
- `ODIN_CLANG_PATH` style backend override behavior is irrelevant to this cut.
- Do not advance to a second backend while the plan-to-linker bridge can alter static archive order.

## Verification

- Release compiler build passed.
- The foreign-library probe builds as `-o:none` and `-o:minimal` objects.
- Repeated and concurrent `-o:none` dumps matched SHA-256
  `10ff84650154599a90e0bf101b34f8304570cdcd4be3e43f1f44bf84c05a5631`.
- The probe preserves two equal-payload `same_payload` declarations and orders equal-priority
  records by package, source filename, source order, and token offset rather than payload.
- Debug, ASan, and no-thread-local requirements remain present in the dump.
