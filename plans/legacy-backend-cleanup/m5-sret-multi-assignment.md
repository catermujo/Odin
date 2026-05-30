# M5 Sret Multi-Result Assignment

## Problem

For one RHS call returning a large indirect result plus direct results, legacy LLVM lowering allocates a
temporary sret buffer but immediately loads the whole buffer into an SSA aggregate. `lb_emit_call` then
builds `lbTupleFix` from those values, and `lb_build_assignment` stores them. A 4,544-byte `marina.Plan`
therefore becomes thousands of GEP/load/`insertvalue` instructions before it is copied to its destination.

## Scope

- Change only legacy LLVM call/assignment lowering in `src/llvm_backend_proc.cpp`,
  `src/llvm_backend_stmt.cpp`, and `src/llvm_backend.hpp` when a declaration change is necessary.
- Add one focused test under `tests/cg/` for a large indirect result and scalar result assigned together.
- Do not modify CG sources, LLVM pass configuration, target-machine settings, or unrelated assignment paths.

## Required mechanism

- Preserve Odin multi-assignment semantics: evaluate the call first, then store every destination.
- Keep each ABI indirect result in temporary address-backed storage through the call.
- Copy that storage to its LHS after the call using the existing LLVM memory-copy mechanism; never rebuild
  a field-by-field SSA aggregate.
- Keep direct ABI results on their existing value path.
- Blank destinations still receive valid ABI storage but do not need a final copy.
- Do not forward an sret pointer directly into a visible LHS: that changes observable assignment timing.

## Acceptance

- The focused fixture passes default legacy at `-o:none` and `-o:minimal`.
- Fixture output is correct for both a large result and its scalar sibling.
- LLVM IR for the caller has a memory copy for the large result and no field-by-field `insertvalue` chain.
- Existing multi-result split-ABI fixture still passes at both levels.
- Release compiler builds and the working tree is left uncommitted for main review.

## Result

- Status: complete.
- Legacy default passes the new large-return witness and five existing multi-result/sret witnesses at both
  `-o:none` and `-o:minimal`.
- The witness IR calls through a temporary sret buffer, then emits one 4,100-byte `llvm.memcpy` to the
  global destination with no `[1025 x i32]` `insertvalue` chain.
- Fresh no-debug AddA single samples: none is 3.81 s (was 4.12 s) and minimal is 4.81 s (was 8.75 s).
  Minimal `main` object emission is 1.951 s (was 6.232 s).
