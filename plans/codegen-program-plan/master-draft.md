# Code Generation Program Plan

- Status: in_progress
- Scope class: large
- Last updated: 2026-08-04

## Planning Horizon

- Current milestone: M6B Linux amd64 native object bootstrap.
- Next checkpoint: opt in to a native backend, emit one deterministic ELF x64
  scalar-leaf object, and prove it valid without executing generated output.
- Deferred detail: semantic operation body contract, non-leaf lowering, data,
  lifecycle, target/ABI expansion, debug/sanitizer parity, and performance.

## Problem

Code generation semantics are split between checker state and LLVM-owned discovery. A second emitter cannot
reliably consume declarations, data, lifecycle roots, ABI facts, or dynamically discovered work.

## Scope

### In scope

- One immutable, backend-neutral `CodeGenProgramPlan` built after checking and before backend setup.
- Deterministic semantic declarations, variable initialization order, lifecycle roots, module ownership, and
  initial procedure work.
- Canonical atomic plan dump through `ODIN_CODEGEN_PLAN_DUMP=<path>`.
- Later milestones: static data, semantic ABI, work queue, backend dispatch, and linker ownership boundary.

### Out of scope

- Replacement value IR or body plan.
- Native emitter implementation.
- LLVM optimization/pass changes.
- Backend-specific data or ABI values inside the plan.

## Dependencies

- Existing `CodeGenGlobalPlan` in `src/codegen_plan.*`.
- Checker semantic order in `CheckerInfo::variable_init_order`, init/fini/test/entry fields, and package data.
- Existing LLVM declaration/body adapter in `src/llvm_backend.cpp`.

## Success Criteria

- Full plan contains deterministic constants, types, globals, procedures, imports, initial bodies, lifecycle,
  entry/test roots, ownership, and requirements without LLVM state.
- Global order equals `CheckerInfo::variable_init_order`.
- Requested plan dumps are atomically written, canonical, and byte-identical across repeated builds.
- Unset dump environment performs no dump allocation, hashing, file I/O, or output.
- Existing LLVM path and corpus behavior remain unchanged.
- `LinkerData` remains independently usable after dispatch milestone.

## Milestones

### M1 - Program-plan spine and dump

- Status: complete.
- Purpose: replace the incomplete global plan with an immutable semantic program-plan shell and prove stable
  construction before LLVM setup.
- Depends on: existing checker semantic order and current plan adapter.
- Files: `src/codegen_plan.hpp`, `src/codegen_plan.cpp`, `src/main.cpp`, `src/llvm_backend.cpp`, focused
  plan-dump fixtures as needed.
- Acceptance criteria:
  - `CodeGenProgramPlan` owns neutral declaration, lifecycle, ownership, import, initial-body, and requirements
    records; no LLVM header, type, value, attribute, module, or generator state leaks in.
  - Every checker global variable is represented in `variable_init_order` with declaration/type/name/attributes
    and initializer classification.
  - Every collection has a defined deterministic planner order before consumption or dump.
  - `ODIN_CODEGEN_PLAN_DUMP=<path>` writes `codegen-plan-v1` first, immediately after complete plan construction
    and before backend setup; unset mode does nothing.
  - Writes use a same-directory temporary then atomic replacement; open/write/close failures are compiler errors.
  - LLVM consumes an adapter with unchanged emitted behavior.
- Verification:
  - Release compiler build.
  - Focused globals/init/fini/test/import fixtures at `-o:none` and `-o:minimal`.
  - Two dump-enabled identical builds; external byte comparison.
  - Dump-unset LLVM object comparison against pre-change control.
- Evidence:
  - `./build_odin.sh release` passes.
  - Dump gates at `/private/tmp/codegen-plan-m1.iqNvQe/gates2.hQeASu`: repeated `-o:none` SHA-256
    `68346e…`, repeated `-o:minimal` SHA-256 `22f38c…`, atomic replacement, failure diagnostics,
    ordinary `-` filename, and no absolute source identity.
  - Pre-M1 and M1 dump-unset main object are identical; dump-set and dump-unset main object SHA-256 is
    `b92655…`.
  - Init ordering dump records `zdep_init`, then `main_init`, cleanup, and entry.
  - `tests/issues/run.sh` passes through the existing suite until the pre-M1 baseline failure at
    `test_issue_build_tag_define_order`; the preserved pre-M1 binary fails identically.
  - Independent contract and LLVM adapter reviews report no remaining issues.

### M2 - Static-data and lifecycle consumption

- Status: complete.
- Depends on: M1.
- Outcome: planner owns `CodeGenDataInit`, global initialization, init/fini/test/entry ordering, and module/package
  ownership; LLVM stops re-reading these semantics from checker state.
- Detail deferred: byte/relocation ownership and exact constant lowering require the M2A lifecycle adapter.
- 2026-08-04 audit: incomplete. Planned runtime-fallback globals retain only a
  boolean; LLVM rereads `DeclInfo::init_expr` and lowers the AST. Session-owned
  runtime-global sources keyed by planned global remain required.

#### M2A - Lifecycle and entry ownership

- Status: complete.
- Purpose: migrate ordered startup, cleanup, test roots, entry root, builtin entry calls, debug entry name, and
  test-wrapper suppression to `CodeGenProgramPlan` without changing global initializer lowering.
- Files: `src/codegen_plan.hpp`, `src/codegen_plan.cpp`, `src/llvm_backend.hpp`, `src/llvm_backend.cpp`, and
  `src/llvm_backend_proc.cpp`.
- Acceptance criteria:
  - Planner records whether a runtime supplied test entry point suppresses wrapper generation.
  - LLVM no longer reads `init_procedures`, `fini_procedures`, `testing_procedures`, or `entry_point` directly.
  - Existing init/fini/test/entry order and emitted object output remain unchanged.
  - Plan dump and focused init/test gates remain byte-stable.
- Verification:
  - Release compiler build.
  - `tests/cg/init_procedure_order` build and `tests/cg/test_entry_wrapper` test with dump.
  - Pre/post object checks at none, minimal, and debug.
- Evidence:
  - `./build_odin.sh release` passes.
  - `tests/cg/init_procedure_order` builds and `tests/cg/test_entry_wrapper` passes 2/2 at `-o:none` and
    `-o:minimal`.
  - Two `-o:none` plan dumps match byte-for-byte, SHA-256 `af4a9c…`.
  - Dump-unset and dump-set main objects match byte-for-byte at none (`92daba…`), minimal (`d68874…`), and
    debug (`c51e15…`). Artifacts: `/private/tmp/codegen-plan-m2a.JCP1EJ`.
  - Independent lifecycle-contract and lifecycle-regression reviews report no issues.

#### M2B - Static-data ownership

- Status: in_progress.
- Purpose: move global initializer semantics from LLVM into plan-owned static-data records, then make LLVM adapt
  those records without reinterpreting Odin AST constants.
- Depends on: M2A.

##### M2B1 - Static-data schema and classification

- Status: complete.
- Files: `src/codegen_plan.hpp`, `src/codegen_plan.cpp`, and one focused dump fixture only if needed.
- Purpose: define explicit zero-fill, bytes, relocation, and runtime-fallback forms; classify every planned global
  without LLVM state or AST pointers in retained plan data.
- Acceptance criteria:
  - A global owns storage size/alignment, initializer payload records, and a runtime-init requirement.
  - Data records distinguish zero-fill, bytes, and symbol relocation; no empty-byte sentinel carries semantics.
  - Existing global declaration facts preserve TLS, readonly, section, visibility, foreign/export/weak, owner, and
    checker initialization order.
  - Planner records only stable semantic symbols/identities in dump output.
  - Existing LLVM generation remains unchanged while this is plan-only.
- Verification: release build; stable plan dumps; focused scalar, aggregate, relocation, TLS, and runtime-global
  compiler fixtures.
- Evidence:
  - `./build_odin.sh release` passes.
  - Existing scalar, aggregate, relocation, runtime-init, and host-TLS global fixtures build.
  - Repeated aggregate plan dumps match, SHA-256 `ef345c…`; dump-unset/set aggregate main objects match,
    SHA-256 `0454aa…`. Artifacts: `/private/tmp/codegen-plan-m2b1.2CIxQc`.
  - Independent schema review reports no issue.

##### M2B2 - Static payloads and backing symbols

- Status: complete.
- Depends on: M2B1.
- Outcome: planner creates byte/zero/relocation payloads for valid scalar, aggregate, string, slice, procedure,
  pointer, and compound constants. Deterministic plan-owned backing symbols cover string and slice storage.
- Open question: reuse a planner-created constant `Entity` for generated backing symbols or add a dedicated neutral
  symbol record while retaining the requested entity relocation target. Resolve with a focused source proof before
  implementation.

###### M2B2A - Zero and scalar payloads

- Status: complete.
- Files: `src/codegen_plan.hpp`, `src/codegen_plan.cpp`, and focused scalar plan-dump coverage only if needed.
- Purpose: populate plan data with explicit full-storage zero-fill records and target-layout scalar byte records;
  retain no `ExactValue`, AST, or LLVM value in global payload records.
- Acceptance criteria:
  - Uninitialized globals produce one zero-fill entry spanning planned storage.
  - Valid scalar constants produce deterministic little/big-endian byte entries matching target layout.
  - Unsupported constant categories remain an explicit runtime fallback until their next dedicated slice.
  - Dump prints record kind, offset, extent, payload bytes, and only semantic relocation identities.
  - Existing LLVM output remains unchanged because no adapter changes in this slice.
- Verification: release build; scalar global dump inspection; repeated dumps and dump-unset/set object comparison;
  existing scalar and runtime-init global fixtures.
- Evidence:
  - `./build_odin.sh release` passes.
  - Scalar and runtime-init globals build at none and minimal.
  - Scalar dump records correct two's-complement `i64`/`i128`, native and endian-qualified floats, enum, bool,
    and nil zero-fill. Repeated dump SHA-256 `9221ce…`; dump-unset/set main object SHA-256 `4bc686…`.
    Artifacts: `/private/tmp/codegen-plan-m2b2a.7TonvB`.

###### M2B2B - Aggregate payloads

- Status: complete.
- Depends on: M2B2A.
- Outcome: arrays, matrices, structs, fixed SOA, bit fields, and padding become ordered byte/zero records using
  target layout. Strings, slices, pointers, and procedure values wait for relocation/backing-symbol support.

##### M2B3 - LLVM static-data adapter and runtime fallback

- Status: complete.
- Depends on: M2B2.
- Outcome: LLVM declares all globals and backing symbols from planned metadata/data, applies linkage/TLS/section/
  readonly attributes, and emits startup stores only for planned runtime fallback in planner order.
- Deferred detail: local-static lowering stays separate until file-global data is proven.

###### M2B3C - Runtime-global semantic source

- Status: complete.
- Depends on: M2B3 planned static declarations and M1.1 `CodeGenSession`.
- Purpose: move file-global runtime initializer AST/`DeclInfo` ownership into
  a session source record. LLVM may lower the source expression, but must obtain
  it through the session rather than rereading a planned declaration's checker
  handles.
- Files: `src/codegen_work_queue.*`, `src/llvm_backend.hpp`,
  `src/llvm_backend.cpp`, focused existing global-runtime fixture only if a
  missing object-only gate needs one.
- Acceptance criteria:
  - Session owns one immutable global source record per planned global, keyed
    by semantic entity and containing planned declaration, entity, `DeclInfo`,
    and initializer expression.
  - LLVM globals retain planned metadata/data; runtime initialization resolves
    entity and expression through the session source only.
  - `lbGlobalVariable` no longer stores a raw `DeclInfo *`.
  - Runtime globals preserve plan order and existing startup lowering.
  - No M3 ABI or M4 procedure-work changes.
- Verification: release build; existing runtime-global object probes at
  `-o:none` and `-o:minimal`; repeated same-output plan/object comparison;
  source audit showing no runtime-global `DeclInfo::init_expr` read in LLVM.
- Evidence: `txzpxnny` seeds one immutable session source for every planned
  global before workers, and LLVM carries it through `lbGlobalVariable`.
  Release build passed. `global_runtime_init.odin` and `global_fixed_dynamic.odin`
  each produced matching plan/object output across three same-output object
  builds in `-o:none` and `-o:minimal`; no generated binary ran. Two independent
  source audits found no remaining runtime-global `DeclInfo`/`init_expr` read.

- Evidence: Planned backing symbols are declared before globals and
  `CodeGenDataInit_Relocation` is recursively adapted into LLVM constants. The
  global aggregate fixture builds/runs at none and minimal without falling back
  to AST constant lowering for planned relocation records.

### M3 - Semantic ABI classification

- Status: complete.
- Depends on: M1.
- Outcome: neutral ABI parts/procedure ABI produced from types and target metrics; LLVM adapts it to LLVM-only
  casts and attributes.
- Detail deferred: per-target split/direct rules must preserve present ABI tests exactly.
- 2026-08-04 completion: LLVM declaration and call lowering consume session ABI
  facts, including planner-owned raw source-to-physical argument layout.

- Evidence: every planned procedure now dumps deterministic source parameter,
  result, environment, and context ABI parts. Source investigation proved that
  a neutral coercion shape is additionally needed for target aggregate ABI
  representation; pass kind alone cannot replace LLVM classification.

#### M3E1 - ABI consumer facts

- Status: complete.
- Depends on: existing `CodeGenProcedureAbi` target classifier and session ABI
  resolver.
- Purpose: make LLVM declaration and call sites consume calling convention,
  sret, closure-environment, context, fixed-parameter count, and C-vararg facts
  from `CodeGenProcedureAbi`, including dynamically constructed procedure types.
- Files: `src/llvm_backend_proc.cpp`; `src/codegen_work_queue.*` only if a
  narrow ABI query helper is necessary; focused existing ABI fixtures only if
  coverage is missing.
- Acceptance criteria:
  - `lb_create_dummy_procedure` and `lb_emit_call` use
    `codegen_session_procedure_abi` for ABI facts.
  - No target ABI reclassification or `llvm_abi.cpp` rewrite.
  - Semantic result types, diverging behavior, and non-ABI procedure metadata
    remain untouched.
  - Existing ABI objects and repeated plan/object output stay unchanged.
- Verification: release build; existing aggregate, closure, C-vararg, and
  indirect-result ABI fixtures at `-o:none`/`-o:minimal`; source audit limited
  to the migrated functions; same-output dump/object comparison.
- Evidence:
  - `xztxxysw` migrated the scoped declaration and call readers to
    `codegen_session_procedure_abi`; release build and ordinary aggregate,
    closure, and C-vararg object probes pass under `-o:none` and `-o:minimal`.
  - M3E1R closes the ignored-argument holes with planner-owned source slots,
    physical LLVM attribute indices, and an ignored fixed-argument fixture.

#### M3E1R - Ignored ABI argument indexing repair

- Status: complete.
- Depends on: `xztxxysw`.
- Purpose: restore planner-owned source-to-physical argument mapping when a
  procedure has compile-time source arguments, ignored fixed ABI arguments, or
  C-varargs.
- Files: `src/codegen_plan.hpp`, `src/codegen_abi.cpp`, plan dump,
  `src/llvm_backend_proc.cpp`, focused CG fixtures.
- Acceptance criteria:
  - The ABI plan records ordered source parameter slots (value, type, constant,
    C-vararg marker) separately from physical ABI parts. Value slots map to a
    contiguous ABI part range; compile-time slots and the C-vararg marker map
    to none.
  - Planner facts identify each physical explicit part's raw source input and
    the first C-vararg source input after type/constant arguments.
  - C-vararg tails begin at that planner-owned source input index, never at a
    physical processed-argument count.
  - Call-site attributes use an emitted LLVM argument index that advances only
    for non-ignored `ft->args`, matching `lb_add_function_type_attributes`.
  - Split-result tuple reconstruction derives its fixed-argument boundary from
    ABI physical-part metadata, never an incidental compact source count.
  - Existing `codegen_abi_plan` no longer triggers the `3 == 4` assertion for
    a polymorphic runtime call; a zero-sized fixed argument before both a
    `#no_alias` pointer and C-vararg tail compiles as an object under
    `-o:none` and `-o:minimal`.
  - Release build, focused ABI objects, and repeat plan/object output pass; no
    generated binary runs.
- Evidence: `xztxxysw` records ordered source slots separately from ABI parts,
  with invalid source references for split results/environment/context. Linux
  amd64 object and IR inspection prove that an ignored `Empty` parameter does
  not shift later `noalias`, `byval`, sret, context, or C-vararg physical
  indices. Definition-side homes and stores close in M3E1S.

#### M3E1S - Definition-side ABI physical-index repair

- Status: complete.
- Depends on: amended `xztxxysw` source-slot mapping.
- Purpose: make procedure declarations and bodies use planner ABI physical
  ordering whenever a source value is absent from LLVM due to `Ignore`.
- Files: `src/codegen_abi.cpp`, `src/llvm_backend_proc.cpp`,
  `src/llvm_backend_stmt.cpp`, focused `tests/cg/ignored_abi_indices.odin`
  regression coverage.
- Acceptance criteria:
  - Declaration attributes come solely from ABI parts and emitted LLVM indices;
    no raw parameter walk, source count, or `return_by_pointer` ABI decision.
  - Callee parameter homes maintain independent ABI-part and physical LLVM
    indices; an ignored source value never shifts later values or context.
  - Named split-result homes find SRet parts by planned `result_index` and
    physical ABI walk, never `original_arg_count`.
  - Split-result stores in statement lowering use the same planned physical
    result-pointer mapping; ignored inputs never redirect a store into context.
  - Linux amd64 object probes cover ignored + NoAlias, attr-bearing call,
    referenced callee value, named split result, C-vararg, type/constant
    source slots, closure, and context at `-o:none`/`-o:minimal`.
  - Release build, default-focused probes, and repeat plan/object output pass;
    no generated binary runs.
- Evidence:
  - `./build_odin.sh release` passes.
  - Linux amd64 `tests/cg/ignored_abi_indices.odin` builds at `-o:none` and
    `-o:minimal`; emitted IR has the post-ignore `noalias` pointer, `byval`
    `Large`, split sret pointer, and Odin context at their correct physical
    LLVM indices.
  - Default-target aggregate, multiple-return, closure, and C-vararg object
    probes pass at `-o:none` and `-o:minimal`; no generated program ran.
  - Contract review verifies callee homes and statement split-result stores use
    planned physical ABI parts; regression review finds only the Linux witness,
    now covered above.

#### M3E1T - Dummy declaration ABI attributes

- Status: complete.
- Depends on: M3E1S.
- Purpose: make dummy LLVM procedure declarations consume the same derived ABI
  function attributes as ordinary declarations.
- Files: `src/llvm_backend_proc.cpp`; existing dummy-procedure fixture or a
  focused object-only probe only if source inspection cannot prove coverage.
- Acceptance criteria:
  - `lb_create_dummy_procedure` initializes its ABI function type then uses the
    shared declaration-attribute helper; no bespoke sret/context index walk.
  - Dummy declarations preserve sret, ByVal, alignment, NoAlias, NoCapture,
    NonNull, and Odin context ABI attributes with ignored parts skipped.
  - Release build, Linux amd64 ignored fixture, and existing ObjC/block or
    generated-helper object probe pass without generated binary execution.
- Evidence:
  - Dummy declarations initialize `abi_function_type` and call the shared
    ABI-attribute helper once; bespoke sret/context emission and duplicate ObjC
    wrapper/block-invoker helper calls are gone.
  - Release build, Linux amd64 ignored fixture at `-o:none`/`-o:minimal`, and
    `tests/cg/objc_block.odin -file -o:none -build-mode:obj` pass.
  - ObjC LLVM IR contains the expected dummy declarations, including sret;
    independent dummy-contract review reports zero findings.
  - Ten same-output Linux amd64 `-o:minimal` builds with
    `ODIN_CODEGEN_PLAN_DUMP` have one object-set SHA-256
    `f1926fdb743ad832e6b9e8b27b0a10dea7319ae1df9a760a3f6c638f630a0ea1`
    and one plan SHA-256
    `dc5ebbdcd96146aab82d2d08a4a2376ea913d230bdf3e05c09a078358e90f774`.

### M4 - Dynamic procedure work queue

- Status: complete.
- Depends on: M1, M2, M3.
- Outcome: deterministic declaration-before-body queue owns initial and dynamically discovered procedures with
  semantic-entity deduplication.

- Evidence: LLVM seeds all plan roots through `CodeGenWorkQueue`, drains every
  declaration before scheduling a root body, and handles body-less imports as
  declarations only. Dynamic discovery remains the active slice.
- 2026-08-04 audit: incomplete. Immutable roots and queue entries carry raw
  `Ast *` bodies, while dynamic discovery remains LLVM `lbProcedure` plumbing.
  Session-resolved, entity-keyed body work is still needed for another backend.

#### M4C - Semantic dynamic body work

- Status: complete.
- Depends on: M3E1T.
- Purpose: make `CodeGenSession` the only semantic owner/resolver of procedure
  bodies. Program-plan and work-queue records carry `{Entity *, reason}` only;
  LLVM requests dynamic work by semantic entity, then adapts drained work to
  its private MPSC queue.
- Files: `src/codegen_plan.hpp`, `src/codegen_plan.cpp`,
  `src/codegen_work_queue.hpp`, `src/codegen_work_queue.cpp`,
  `src/llvm_backend.hpp`, `src/llvm_backend.cpp`,
  `src/llvm_backend_general.cpp`, `src/llvm_backend_proc.cpp`, and
  `src/llvm_backend_stmt.cpp`.
- Acceptance criteria:
  - `CodeGenBodyRequest` and `CodeGenWorkQueueEntry` retain no `Ast *`; body
    requests deduplicate only by canonical semantic procedure entity.
  - `CodeGenProcedureSemanticSource::body` is the sole semantic AST-body
    authority. `codegen_session_request_body` resolves it internally and does
    not queue a bodyless foreign/import procedure.
  - Every initial, nested, anonymous, missing/specialized, and cross-module
    dynamic request enters via session `{entity, reason}` API; no LLVM
    `lbProcedure *` or its `body` field drives semantic scheduling.
  - Declaration-before-body, initial-root order, finalized dynamic-batch order,
    unspecialized-polymorphic skips, cross-module declaration ownership, and
    bodyless foreign behavior remain unchanged.
  - LLVM remains free to cache a resolved body on `lbProcedure` and to use MPSC
    after a semantic request drains; MPSC mechanics are not redesigned.
- Verification:
  - Release compiler build.
  - Object-only `codegen_abi_plan`, `native_closure_abi`, and Linux amd64
    `c_vararg_runtime_string` probes at `-o:none` and `-o:minimal`.
  - Three same-output closure object builds with plan dumps; compare plan and
    object hashes. No generated binary runs.
- Evidence:
  - `yxmoztwy` removes `Ast *` from `CodeGenBodyRequest` and
    `CodeGenWorkQueueEntry`; session resolves body presence from its semantic
    source and LLVM dynamic callers request body work by entity.
  - Release build and six object-only ABI/closure/Linux-C-vararg probes pass.
    No generated program ran.
  - M4C contract review's `body_kind` note is not a retained-AST violation:
    planner construction may inspect checked AST to materialize immutable enum
    metadata; the plan/queue retain no AST and queue resolution remains session
    owned.
  - P1 replaces pointer-equality import dedup with the semantic requester and
    procedure comparisons used by the import sort. Three `-o:none` and ten
    `-o:minimal` closure object builds now have identical plan/object manifests;
    `default_random_generator_proc -> copy_slice` occurs exactly once per dump.

- M4 status: complete. M4A queue core, M4B root consumption, M4C semantic body
  ownership, and dynamic scheduling now form one entity-keyed,
  declaration-before-body session contract.

### M5 - Backend dispatch and linker boundary

- Status: complete.
- Depends on: M2, M3, M4.
- Outcome: `main.cpp` validates one plan before backend setup; LLVM becomes `CodeGenBackend`; `LinkerData` no
  longer requires `lbGenerator` inheritance.

- Evidence: dispatch schema and LLVM backend implementation exist; main dumps,
  validates, initializes standalone linker data, then calls backend generation.
  `lbGenerator` now holds a linker pointer rather than inheriting `LinkerData`.
  2026-08-04 audit confirms generic select/validate/session/linker/generate
  dispatch and shared linker stage. LLVM-only backend selection is expected
  until a second backend kind is introduced.

### M6 - Fast native backend

- Status: in_progress.
- Depends on: M1-M5.
- Outcome: retain LLVM while adding a correctness-first native object backend
  for `-o:none` and `-o:minimal`; eventually cover Windows/Linux/macOS on x64
  and arm64, with ordinary debug and sanitizer contracts.
- Constraints: feature correctness before performance; native path must fail
  clearly for unsupported behavior, preserve LLVM fallback/selection policy,
  and never weaken LLVM output.

#### M6A - Native-backend reference map

- Status: complete.
- Purpose: map the old proven native-emitter implementation to current
  `CodeGenProgramPlan`, `CodeGenSession`, backend dispatch, linker data, and
  target requirements. Select one independently verifiable first transplant
  slice without importing old LLVM/CG-IR coupling.
- Verification: read-only source map with exact paths/symbols, data ownership,
  target/ABI prerequisites, first-slice acceptance criteria, and object-only
  validation commands.
- Evidence: `m6a-native-reference-map-handoff.md` maps twenty old native
  reference components to current ownership. It identifies explicit backend
  selection, backend artifact destination, and later semantic-body interface as
  the only missing foundation contracts.

#### M6B - Linux amd64 native object bootstrap

- Status: implementation_complete_remote_inspection_pending.
- Depends on: M6A.
- Purpose: introduce an explicit experimental native backend and prove a
  deterministic ELF64 x86-64 relocatable object for a scalar integer/pointer
  leaf procedure. This establishes selection, artifact, ABI, instruction, and
  object-writer seams without importing old CG/LLVM IR.
- Files: command/build settings, `src/codegen_plan.*`,
  `src/codegen_backend.cpp`, `src/linker.cpp`, new native backend/private ELF
  x64 files, and one focused compiler fixture.
- Acceptance criteria:
  - `-backend:native` is immutable plan input; default remains LLVM.
  - Native validator accepts only Linux amd64 object builds at `-o:none` or
    `-o:minimal`, without debug/sanitizers/TLS/globals/data/lifecycle/entry
    wrapper/imports, and emits clear diagnostics for every excluded feature.
  - Driver/linker provides native generation one deterministic object path;
    native appends exactly that artifact without reading mutable build context.
  - Scalar direct integer/pointer parameter/constant add/subtract/direct return
    lowers from session semantic source into a valid ELF64 x86-64 `ET_REL`
    `.text` object with one function symbol and no relocation.
  - Repeated objects hash identically. `readelf` and disassembly inspection on
    `local-arch` confirm header, sections, symbol, relocations, and instructions;
    no generated object is linked or run.
- Deferred: all broader language forms, external/direct calls, data/global
  emission, runtime/lifecycle, non-object modes, other targets, debug and
  sanitizer support. Each must fail closed in this bootstrap.
- Local evidence: plain `./build_odin.sh` passes; none/minimal native objects
  have identical SHA-256 `83cabbeac558965c1ed2afc48cf7c1caf9070a5680a51ba4a0d74b9cfced8910`.
  `llvm-readelf` confirms ELF64 x86-64 `ET_REL`, `.text`, `.symtab`, no
  relocations, and the global scalar-leaf symbol; `llvm-objdump` confirms the
  expected parameter moves, add, subtract, and return. Debug, sanitizer,
  global, and foreign-procedure probes reject without changing a sentinel
  output. `local-arch` was unreachable (`No route to host`) during remote
  verification, so its inspection remains the only M6B gate.

## Decisions

- Dump is only `ODIN_CODEGEN_PLAN_DUMP=<path>`; `-` has no special behavior.
- Dump is canonical text, not an in-compiler hash API. External byte comparison is the gate.
- Atomic dump replacement occurs in destination directory so rename stays atomic.
- M1 captures semantic facts only. It does not introduce replacement value IR or LLVM data.
- M3 must add a backend-neutral ABI coercion shape before LLVM consumption;
  keeping direct/indirect/ignore alone would leave target classification inside
  the backend.
- Native bootstrap is opt-in as `-backend:native`; automatic selection for
  `-o:none`/`-o:minimal` waits for full platform/flag correctness coverage.
- Native bootstrap starts Linux amd64 ELF because its old writer is discrete
  and `local-arch` permits object inspection. This is a proof slice, not a
  reduced platform promise.

## Blockers and Follow-ups

- No active blocker. M6 target ordering and first object-writer scope await the
  M6A reference map.
