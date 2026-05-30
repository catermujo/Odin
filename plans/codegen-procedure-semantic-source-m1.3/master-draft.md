# M1.3 - Procedure semantic-source contract

- Status: complete
- Scope class: medium
- Last updated: 2026-08-04

## Planning horizon

- M1.3 is accepted. Next foundation checkpoint: body-level planning. Neither
  AST-free body IR nor a second backend is included here.

## Problem

LLVM still reconstructs procedure metadata from `Entity::Procedure` and local
AST walks. This duplicates semantic decisions and makes static and dynamically
discovered procedures follow different paths.

## Scope

### In scope

- Snapshot procedure metadata required by LLVM into planner-owned declarations.
- Keep `DeclInfo` and AST handles in `CodeGenSession` semantic-source records.
- Resolve planned declaration and semantic source through one session API.
- Route static, generic, nested, anonymous, and deferred procedures through it.
- Preserve deterministic declaration-before-body work-queue behavior.
- Prove behavior with focused object builds and deterministic plan/object dumps.

### Out of scope

- AST-free body IR.
- New backend implementation.
- Changing procedure ABI behavior or backend optimization policy.

## Dependencies

- M1.2D requirement contract (`zonlvqsx`) is sealed.
- `CodeGenProgramPlan`, `CodeGenSession`, and LLVM work queue already exist.

## Success criteria

- LLVM no longer snapshots or mutates procedure semantic metadata locally.
- Session resolver supplies the same planned declaration and semantic source for
  static and dynamically discovered procedures.
- Queue declarations remain ahead of bodies and dynamic ordering remains stable.
- Generic imports, nested/anonymous procedures, deferred procedures, and
  cross-module declarations compile in focused object probes.
- Repeated plan dumps and objects are byte-identical.

## Milestones

### M1.3A - Semantic source and metadata

- Status: complete
- Files: `src/codegen_plan.*`, `src/codegen_work_queue.*`, LLVM procedure
  consumers, focused CG fixtures when coverage is missing.
- Acceptance: procedure declaration values contain LLVM-consumed semantic
  metadata; the session maps each entity to exactly one source record holding
  its declaration, `DeclInfo`, procedure literal, type expression, and body.
- Evidence: `./build_odin.sh release` passed. `CodeGenProcedureDecl` now
  snapshots body kind, inlining/tailing, tags, fast-math, ObjC data, deferred
  procedure and scope-exit data, generated-polymorphic status, entry-only and
  memcpy-like flags.

### M1.3B - LLVM migration

- Status: complete
- Depends on: M1.3A resolver.
- Evidence: direct `Entity::Procedure` reads are absent from every
  `llvm_backend*.cpp` consumer. Procedure link-name reads and writes use the
  session lock. Anonymous setup and dynamic declaration creation both resolve
  through the same session source record.

### M1.3C - Determinism and contract gates

- Status: complete
- Depends on: M1.3B.
- Evidence: object probes passed for generic cross-module callbacks,
  nested/anonymous literals, normal defers, and scope-exit lowering. The
  cross-module callback fixture was built twice with one temporary entry point:
  its plan dump and all 31 objects matched byte-for-byte. Plan SHA-256:
  `58795a70c6d49673eceb4477baf8e60211692c00adf628b33c11aec3c07b9f04`.
  The temporary entry also ran and checked direct, callback, global callback,
  and generic callback results; standalone anonymous, defer, and scope-exit
  executables each ran successfully. Anonymous lowering now elects, registers,
  and release-publishes its winner under `dynamic_procedures_mutex`; losing
  candidates never allocate an entity or enter a session map. Anonymous names
  derive from package name, package-relative source name, source offset, and
  canonical procedure type hash.

  Release build passed. `tests/cg/proc_lit.odin` produced identical objects in
  ten same-output builds under both `-o:none` and `-o:minimal`, including
  `runtime-error_checks.o`. Nested/anonymous, defer, and scope-exit executables
  passed in both modes. Cross-module callback and immutable-global tests passed
  in both modes (4/4 test runs).

  Final repair `xtwlolwy` registers the private winner before release
  publication. Two independent source audits found no remaining publication or
  metadata-read finding. Master reran four same-output `proc_lit.odin` object
  builds in each of `-o:none` and `-o:minimal`; all generated objects and plan
  dumps matched within each mode. No generated binary ran.

## Decisions

- Procedure values belong in `CodeGenProcedureDecl`; AST and `DeclInfo` remain
  session-owned because code generation still lowers bodies from semantic AST.
- Dynamic records use the same declaration construction and session map as
  static records, avoiding a second LLVM-only metadata path.
- Generated local static names use source position plus a deterministic
  per-procedure sequence, rather than checker-assigned `Entity::id`, because
  those IDs vary across parallel checker runs.

## Blockers and follow-ups

- The broad `tests/core/normal.odin` command is not a codegen signal on this
  macOS host: after JSON tab normalization it reaches unrelated test API errors,
  and without style vetting the linker asks for the Windows `Ole32.lib`.
