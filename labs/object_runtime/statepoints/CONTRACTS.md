# Executed compiler-managed relocation boundary

This required X-T74-011 experiment is a separate x86-64 Linux System V ABI
adapter. It is not portable ISO C stack introspection and does not turn arbitrary
C locals into traced roots. The pinned LLVM tool version, emitted IR, assembly,
stack maps and execution are the evidence. Read the LLVM 21.1 Statepoints
sections Abstract Machine, Stack Map Format, Safepoint Semantics and Verification,
and RewriteStatepointsForGC; read StackMaps, Stack Map Format and Location Types.
LLVM represents relocation with new SSA values and records base/derived pairs;
the runtime must update every usable mapped copy. A scalar with identical bits
is still a scalar. Compiler rematerialization can reduce physical root slots.

## Inputs and implementation

`source.ll` receives a managed allocation base in address space 1, an offset
0..7, and an integer whose value equals the original base address. It computes
two derived references, calls `gc_poll`, then reads both and adds the scalar.
Implement `relocate_roots` in `runtime.c`. `poll.S` passes the caller stack pointer
and actual return address to that function with the required stack alignment.
`maps.ld` exposes the emitted `.llvm_stackmaps` region. `driver.c` owns both
16-byte static arrays; only the first eight bytes form the logical object.
No original storage is freed, so the deliberately broken controls demonstrate
wrong values without dangling C object accesses. The extra byte is a guard.

Accept only the actually emitted subset: little-endian version 3, one function,
one record, no constant pool, correct call-site return address, three zero
constant prefix locations, then base/derived pairs of 8-byte indirect stack
locations using DWARF register 7. Reject other encodings rather than guessing.
Check all map extents and stack offsets. Read and validate every original pair
before updating any slot, because multiple pairs can share a base location.
Stage each unique destination once. Every original base must identify
`old_space`; every derived offset is 0..7. Copy the logical object to new_space,
clear old_space, then update each mapped derived slot to new_space plus its
original offset. Preserve scalar registers and all ABI-preserved state. This
laboratory uses compiler-defined stack slot mutation; ISO C alone does not
authorize the relocation ABI or prove provenance after such writes.

The harness resets bytes to 10..17 for every offset. The exact post-poll result
is `scalar + 27 + offset`; old bytes are zero and the guard byte is 99. Require
eight successful polls. The default rewrite and the rewrite with
`-spp-rematerialization-threshold=0` must both execute. Record their actual slot
counts rather than requiring a universal optimizer layout. A mode that omits
updates and a mode that shifts the destination by one must each fail the
specific value assertion with a normal nonzero exit, not a signal.

## Falsification and transfer

Inspect pre/post IR: `gc.statepoint`, `gc.relocate`, actual `gc-live` operands,
and which derived references are rematerialized. Replace post-safepoint uses of
the relocated base with the old SSA base in a separate invalid IR artifact.
Run the dedicated `verify<safepoint-ir>` pass and retain the exact diagnostic;
ordinary structural IR verification is not equivalent. The observed LLVM
21.1.8 pass diagnoses an illegal unrelocated value even though the 21.1.0 prose
describes a limitation of verifier support. Record that version-specific result
instead of treating prose as an execution receipt. The verifier may abort;
the runner suppresses core files in that child process only.

Derive the difference between compiler root registration and the CVM adapter's
explicit root addresses. Then change the IR to a second live managed allocation
and derive a multi-object forwarding map before extending the runtime parser.
Do not label the one-function parser a production collector. Foreign-thread
handshake, asynchronous collection, unwind tables, arbitrary base locations,
interior references outside the object, other architectures, and physical
measurements require distinct implementations and evidence. All executable
targets are original local programs with synthetic bytes.

## Assessment

Level 1 reproduces emitted text. Level 2 explains and executes correct updates,
scalar preservation, both optimizer layouts, and the two harmless value
controls. Level 3 derives and implements the changed multi-object map and
explains the compiler/ABI/ISO C enforcement boundary without a safety-equivalence
claim. Keep the learner implementation separate from the assessor runtime;
the source-bound runner also checks that the supplied learner stub fails the
intended unimplemented assertion after successful compilation.
