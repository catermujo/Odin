# Legacy Backend Cleanup

- Status: in_progress
- Scope class: large
- Last updated: 2026-08-02

## Planning Horizon

- Current milestone: M5 - Compile throughput.
- Next checkpoint: compare final LLVM IR structural workload for the proven minimal object-emission gap.
- Deferred detail: the first throughput repair depends on IR evidence; no LLVM pass change is assumed.

## Problem

Legacy LLVM codegen owns declaration discovery, procedure scheduling, and LLVM emission in one backend. That blocks a second emitter and hides redundant scheduling work.

## Scope

### In scope

- Extract initial procedure-body work from LLVM module queues into a deterministic, backend-neutral plan.
- Keep LLVM as the only emitter and preserve parallel body lowering.
- Maintain default-legacy correctness at `-o:none` and `-o:minimal`.

### Out of scope

- Native emitter implementation.
- LLVM pass changes.
- Dynamic nested/helper procedure scheduling; M3 only owns the initial reachable body frontier.

## Dependencies

- M1 `qwuurpvk`: global entity planning is backend-neutral.
- M2 `svmxllov`: all initial owners and named imports are declared before workers.
- `CodeGenGlobalPlan` and `lb_module_of_entity` establish deterministic entity ownership.

## Success Criteria

- Initial procedure work is an explicit `Entity *` plan with no LLVM type in its contract.
- LLVM module queues are populated only by adapting that plan after declarations exist.
- Global procedure ordering follows the source-sorted plan.
- Release compiler, strict scheduler/closure fixtures, and threaded native SIMD checks pass at `-o:none` and `-o:minimal`.

## Milestones

### M1 - Global planning

- Status: complete
- Change: `qwuurpvk`.

### M2 - Declaration barrier

- Status: complete
- Change: `svmxllov`.

### M3 - Initial procedure work plan

- Status: complete
- Depends on: M1, M2.
- Purpose: make the initial body frontier backend-neutral while leaving lowering and dynamically generated procedures in LLVM.
- M3A status: complete. Initial top-level procedures are selected in `codegen_build_global_plan`; LLVM routes them to module queues. Dynamic equality, nested, anonymous, and recovery paths remain separate.
- Current slice M3B: add sorted initial-procedure work to `CodeGenGlobalPlan` and adapt it to the already-declared LLVM owner queue.
- Files: `src/codegen_plan.hpp`, `src/codegen_plan.cpp`, `src/llvm_backend.cpp`.
- Acceptance criteria:
  - The plan has a sorted, backend-neutral `Entity *` initial-procedure list.
  - Declaration staging no longer enqueues bodies directly from module-local global lists.
  - The adapter queues each planned procedure in its resolved owner module after owners/imports exist.
  - Dynamic queues retain current behavior.
- Verification: release build; strict threaded scheduler, lazy-materialization, and closure object gates; 100 threaded native SIMD object builds per none/minimal; `git diff --check`.
- Evidence: release build; six focused strict gates; native SIMD 100/100 per none/minimal; independent contract check; independent native SIMD 50/50 per level with ten matching object hashes; master strict scheduler builds at both levels; `git diff --check`.

### M4 - Emitter boundary

- Status: complete
- Depends on: M3.
- Outcome: frontend work can feed LLVM and a future native emitter without the legacy LLVM scheduler being the producer.
- M4A status: complete. `CodeGenGlobalPlan` already contains the checked constants, declarations, imports, and initial bodies without LLVM state.
- Current slice M4B: create that plan once in `main.cpp`, then pass it immutably to the LLVM entry point.
- Files: `src/main.cpp`, `src/llvm_backend.cpp`.
- Acceptance criteria:
  - `main.cpp` builds `CodeGenGlobalPlan` after checker/cache completion and before emitter setup.
  - `lb_generate_code` and declaration staging consume the immutable plan; LLVM no longer constructs it.
  - The plan is not stored in `lbGenerator` and LLVM mapping/lowering remains unchanged.
  - A future native emitter can consume the same plan directly.
- Verification: release build; default-legacy scheduler, lazy-materialization, closure, masked-memory, and threaded native SIMD object gates at none/minimal; `git diff --check`.
- Evidence: `soqoovsy` passes the plan from main. Follow-up moves plan construction before LLVM generator setup; release build; six default-legacy fixture builds; native SIMD 100/100 master and 50/50 independent per level; masked-memory both levels; one planner caller in main; `git diff --check`.

### M5 - Compile throughput

- Status: in_progress
- Depends on: M4.
- Outcome: measure default legacy and CG on add_a at `-o:none` and `-o:minimal`, then define the first evidence-backed speed fix.
- Measurement unit: the Makefile's `$(ODIN) build src` recipe only, with its collection, vet, debug, dynamic-map, and linker flags. Prebuilt Make prerequisites are reused; their C/assets time is out of scope.
- Samples: one cold output build per backend and level, as agreed. Use unique `/private/tmp` outputs and external wall-clock timing; no cache flags.
- Acceptance criteria: all four builds link successfully and record command, compiler mode, optimization level, exit code, and wall time.
- Evidence:
  - Default legacy (`odin_b`): none 4.87 s; minimal 32.86 s.
  - CG (`odin_a`, `ODIN_CG_ONLY=1`): none 2.56 s; minimal 3.42 s.
  - Legacy is 90.2% slower at none and 860.8% slower at minimal by wall time.
  - The earlier `ODIN_CG_ONLY=1` cells run against `odin_b` are invalid controls: that branch has no CG selector.
  - Minimal detailed CSVs: `/private/tmp/m5-add-a-20260802/legacy-minimal-detailed.csv` and
    `/private/tmp/m5-add-a-20260802/cg-minimal-detailed.csv`.
  - At minimal, both compile 153 used modules. Legacy spends 24.277 s in LLVM object generation and
    3.601 s in module passes; CG spends 1.047 s and 1.084 s respectively. Legacy procedure scheduling
    itself is not the dominant timed cost.
  - Legacy final IR with `-debug` has 278,391 `#dbg_value`, 24,687 `#dbg_declare`, 61,609 local-variable
    metadata nodes, and 179,189 location nodes; CG has none of those records. A live legacy sample is
    dominated by `DwarfDebug::buildLocationList` sorting `DbgValueLoc`.
- Removing `-debug` is only a control, not a candidate fix: legacy total falls to 8.443 s and object
    emission to 6.175 s, but this loses all debug information.
  - The no-debug legacy IR is smaller than CG final IR (87.46 MiB versus 94.19 MiB), has fewer blocks
    (77,587 versus 85,701), and nearly equal instruction lines (1,008,563 versus 1,000,604). The residual
    is therefore not explained by IR volume or basic-block count.
  - A no-debug residual sample is inside normal AArch64 SelectionDAG lowering. `rssvqtzu` adds opt-in
    `ODIN_LLVM_PROFILE` worker timing, with no scheduling change.
  - That profile identifies one real tail, not a hash-order balancing problem: legacy `main` takes 6.208 s;
    its next module takes 0.428 s. CG's matching `main` takes 0.983 s.
  - Fresh no-debug AddA cells: legacy/CG wall time is 4.12/2.74 s at none and 8.75/3.44 s at minimal.
    The minimal legacy `main` object worker takes 6.232 s, so debug metadata is not the residual cause.
  - Standalone LLVM 22.1.8 `llc -O0` on the final no-debug `main.ll` files takes 6.66 s for legacy and
    1.50 s for CG with the same target triple. Target-machine creation, module passes, and object-emission
    API are shared; the remaining cost is generated IR.
  - LLVM time tracing identifies one procedure: `main::markov_marina_lab_configure` takes 4.29 s in legacy
    AArch64 instruction selection but 1.06 ms in CG. Its legacy IR has 6,874 instructions, 2,171
    `insertvalue`s, 2,162 loads, and 2,198 GEPs; CG has 497 instructions, 33 `insertvalue`s, 42 loads,
    and 61 GEPs.
  - Root mechanism: multi-result assignment of an indirect `marina.Plan` return loads the complete sret
    buffer into an SSA aggregate, rebuilding every field with `insertvalue`, before storing it. CG keeps
    the result address-backed and copies its 4,544-byte buffer. The legacy repair must retain the temporary
    sret buffer and copy it after the call; forwarding sret to a visible destination is not valid because
    it would change multi-assignment timing.
  - Legacy local/parameter debug emission is centralized in `lb_add_debug_local_variable` and
    `lb_add_debug_param_variable` (`src/llvm_backend_debug.cpp`). The separate location path remains in
    `lb_debug_location_from_token_pos`. CG ignores debug-location nodes and clears function subprograms;
    it has no debug-info parity.
- First diagnosis: compare legacy and CG `main` module final IR to identify the remaining target-lowering
  difference. Any reduced-debug mode must preserve line tables and be separately scoped.

## Decisions

- M3 plans only initial reachable procedures; nested, anonymous, and generated helpers remain explicit deferred LLVM work.
- Preserve per-module worker ownership; do not replace it with a global lock.
- Use `Entity *` and source ordering at the planning boundary; do not leak `lbProcedure` or LLVM handles.

## Blockers and Follow-ups

- The old strict corpus command was not preserved by the rebase. Direct scheduler, lazy-materialization, closure, masked-SIMD, and 100-run native SIMD gates are available.
- Legacy object bytes are already nondeterministic upstream, including with one worker. Upstream also crashes under the native-SIMD two-worker control. M4 must preserve successful threaded builds, not introduce an object-hash gate.
